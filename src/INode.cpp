#include "INode.h"
#include "FileSystem.h"
#include "File.h"
#include "Debug.h"
#include <cstring>
#include <ctime>
#include <iostream>

DiskINode::DiskINode()
{
    mode = dirLinkNum = fileSize = lastEditTime = lastViewTime = 0;
    userMode = __none__ = -1;
	memset(addr, 0, sizeof(addr));
}

DiskINode::~DiskINode()
{
    
}

INode::INode()
{
    
}

INode::~INode()
{
    
}

int INode::IRead(IOParameter para)
{
    flag |= IACC;
    int st = para.offset, ed = std::min(para.offset + para.count, fileSize);
    Debug(4, "IREAD: Reading %d bytes from offset %d, inodeid = %d, filesize=%d\n", para.count, para.offset, this->iNodeNo, this->fileSize);
    int lbn = st / FileSystem::BLOCK_SIZE, rbn = (ed - 1) / FileSystem::BLOCK_SIZE, rcnt = 0;
    for(int i = lbn; i <= rbn; i++)
    {
        int blkno = BMap(i);
        if(blkno == -1) continue;
        Buffer *buf = bufferManager.BRead(blkno);
        int stn = i == lbn ? para.offset % FileSystem::BLOCK_SIZE : 0;
        int edn = i == rbn ? ed % FileSystem::BLOCK_SIZE : FileSystem::BLOCK_SIZE;
        memcpy(para.base + rcnt, buf->addr + stn, edn - stn);
        rcnt += edn - stn;
    }
    return rcnt;
}


int INode::IWrite(IOParameter para)
{
    Debug(4, "IWRITE: Writing %d bytes to offset %d, inodeid = %d, filesize=%d\n", para.count, para.offset, this->iNodeNo, this->fileSize);
    flag |= IUPD;
    int st = para.offset, ed = para.offset + para.count;
    int lbn = st / FileSystem::BLOCK_SIZE, rbn = (ed - 1) / FileSystem::BLOCK_SIZE, wcnt = 0;
    for(int i = lbn; i <= rbn; i++)
    {
        int blkno = BMap(i);
        if(blkno == -1) continue;
        Buffer *buf = i == lbn || i == rbn ? bufferManager.BRead(blkno) : bufferManager.GetBlk(blkno);
        int stn = (i == lbn ? para.offset % FileSystem::BLOCK_SIZE : 0);
        int edn = (i == rbn ? (para.offset + para.count) % FileSystem::BLOCK_SIZE : FileSystem::BLOCK_SIZE);
        memcpy(buf->addr + stn, para.base + wcnt, edn - stn);
        wcnt += edn - stn;
        i == lbn || i == rbn ? bufferManager.BDWrite(buf) : bufferManager.BWrite(buf);
    }
    fileSize = std::max(fileSize, para.offset + para.count);
    return wcnt;
}

int INode::BMap(int lbn)//失败返回-1
{
    Buffer *buf, *nbuf;
    auto getDoneAddr = [&](int index){
        if(addr[index] == 0 && (buf = fileSystem.Alloc()) != NULL){
            this->addr[index] = buf->blkno;
            this->flag |= IUPD;
        }
        else if(addr[index] == 0) return false; return true;
    };
    auto getDoneBlkAddr = [&](int index){
        if(((int*)nbuf->addr)[index] == 0 && (buf = fileSystem.Alloc()) != NULL){
            ((int*)nbuf->addr)[index] = buf->blkno;
            nbuf->flag |= IUPD;
        }
        else if(((int*)nbuf->addr)[index] == 0) return false; return true;
    };
    Debug(3, "BMAP: Mapping lbn %d, inodeid = %d", lbn, this->iNodeNo);
    if(lbn >= 128 * 128 * 2 + 128 * 2 + 6) return 0;
    int index = lbn < 6 ? lbn : (lbn < 128 * 2 + 6 ? (lbn - 6) / 128 + 6 : (lbn - 262) / (128 * 128) + 8);
    Debug(3, "BMAP: NORMAL index = %d\n", index);
    if(!getDoneAddr(index)) return -1;
    if(lbn < 6) return addr[index];//普通文件
    nbuf = bufferManager.BRead(addr[index]);
    index = lbn < 262 ? (lbn - 6) % 128 : (lbn - 262) % (128 * 128) / 128;
    if(!getDoneBlkAddr(index)) {Debug(3, "BMAP: NO Large File Index."); return -1;}
    Debug(3, "BMAP: LARGE FILE block = %d, index = %d\n", ((int*)nbuf->addr)[index],index);
    if(lbn < 262) return ((int*)nbuf->addr)[index];//大型文件
    nbuf = bufferManager.BRead(((int*)nbuf->addr)[index]);
    index = (lbn - 262) % 128;
    if(!getDoneBlkAddr(index)) return -1;
    return ((int*)nbuf->addr)[index];//巨型文件
}

DiskINode INode::WriteDiskINode()
{
    DiskINode diskINode;
    diskINode.mode = mode;
    diskINode.dirLinkNum = dirLinkNum;
    diskINode.userMode = userMode;
    diskINode.__none__ = __none__;
    diskINode.fileSize = fileSize;
    memcpy(diskINode.addr, addr, sizeof(addr));
    diskINode.lastViewTime = lastViewTime;
    diskINode.lastEditTime = lastEditTime;
    Debug(7, "INode::WriteDiskINode Writing inode %d, filesize = %d, userMode = %d\n", iNodeNo, fileSize, userMode);
    return diskINode;
}

void INode::readDIskINode(DiskINode diskINode)
{
    mode = diskINode.mode;
    dirLinkNum = diskINode.dirLinkNum;
    userMode = diskINode.userMode;
    __none__ = diskINode.__none__;
    fileSize = diskINode.fileSize;
    memcpy(addr, diskINode.addr, sizeof(addr));
    lastViewTime = diskINode.lastViewTime;
    lastEditTime = diskINode.lastEditTime;
    Debug(7, "INode::readDIskINode Reading inode %d, filesize = %d, userMode = %d, mode=%d\n", iNodeNo, fileSize, userMode, mode);
}

void INode::SetFileUserMode(int flag){
    userMode = flag;
    this->flag |= IUPD;
    mode = INodeFlag::IFILE;
}

bool INode::HasFileUserMode(int flag)
{
    return userMode & flag;
}

void INode::IUpdate(int time)
{
    Debug(4, "IUPDATE: Updating inode %d, filesize = %d, userMode=%d, flag = %d\n", iNodeNo, fileSize, userMode, flag);
    if(!flag) return;
    if(flag & IACC) lastViewTime = time;
    if(flag & IUPD) lastEditTime = time;
    flag = 0;
    Buffer *buf = bufferManager.BRead(FileSystem::INODE_ZONE_START_SECTOR + iNodeNo / 8);
    DiskINode* diskINode = (DiskINode *)(buf->addr + (iNodeNo % 8 * sizeof(DiskINode)));
    DiskINode ndiskINode = WriteDiskINode();
    memcpy(diskINode, &ndiskINode, sizeof(DiskINode));
    bufferManager.BWrite(buf);
}

void INode::ITrunc()
{
    Debug(4, "ITRUNC: Truncating inode %d, filesize = %d\n", iNodeNo, fileSize);
    if(this->mode == INodeFlag::IDIR) {
        ((DirINode*)this)->DeleteAllNode();
    }
    for(int i = 0; i < 6; i++){
        if(addr[i] != 0){
            fileSystem.Free(addr[i]);
            addr[i] = 0;
        }
    }
    for(int i = 6; i < 8; i++){
        if(addr[i] != 0){
            Buffer *buf = bufferManager.BRead(addr[i]);
            for(int j = 0; j < 128; j++){
                int addr = *(int *)(buf->addr + j * sizeof(int));
                if(addr != 0) fileSystem.Free(addr);
            }
            fileSystem.Free(addr[i]);
            addr[i] = 0;
        }
    }
    for(int i = 8; i < 10; i++){
        if(addr[i] != 0){
            Buffer *buf = bufferManager.BRead(addr[i]);
            for(int j = 0; j < 128; j++){
                int addr = *(int *)(buf->addr + j * sizeof(int));
                if(addr != 0){
                    Buffer *nbuf = bufferManager.BRead(addr);
                    for(int k = 0; k < 128; k++){
                        int addr = *(int *)(nbuf->addr + k * sizeof(int));
                        if(addr != 0) fileSystem.Free(addr);
                    }
                    fileSystem.Free(addr);
                }
            }
            fileSystem.Free(addr[i]);
            addr[i] = 0;
        }
    }
    this->fileSize = 0;
    this->mode = INodeFlag::IFREE;
    this->flag |= IUPD;
}

void INode::Clean()
{
    mode = dirLinkNum = fileSize = lastEditTime = lastViewTime = 0;
    userMode = __none__ = -1;
	memset(addr, 0, sizeof(addr));
}

void INode::ICopy(Buffer *buf, int iNumber)
{
    DiskINode *diskINode = (DiskINode *)(buf->addr + (iNumber % 8 * sizeof(DiskINode)));
    readDIskINode(*diskINode);
}

INodeManager::INodeManager()
{

}

INodeManager::~INodeManager()
{

}

INode *INodeManager::IGet(int no)
{
    INode *ret = nullptr;
    if(IsLoaded(no)){
        ret = iNodeMap[no];
        ret->usingCount++;
    } else {
        Buffer *buf = bufferManager.BRead(FileSystem::INODE_ZONE_START_SECTOR + no / 8);
        if((ret = GetFreeINode()) == nullptr){
            std::cout << "INodeTable Out of Memory!" << std::endl;
            return nullptr;
        }
        ret->iNodeNo = no;
        ret->mode = 1;
        ret->usingCount++;
        ret->ICopy(buf, no);
        iNodeMap[no] = ret;
    }
    Debug(3, "IGET: Getting inode %d=%d, filesize = %d, linknum=%d\n", no,ret->iNodeNo, ret->fileSize, ret->dirLinkNum);
    return ret;
}

void INodeManager::IPut(INode *node)
{
    Debug(3, "IPUT: Putting inode %d, filesize = %d,usingCount=%d, linknum=%d\n", node->iNodeNo, node->fileSize,node->usingCount, node->dirLinkNum);
    if(node->usingCount == 1){
        iNodeMap.erase(node->iNodeNo);
        if (node->dirLinkNum <= 0) {
            node->mode = 0;
            node->userMode = 0;
            node->flag |= IUPD;
            node->ITrunc();
            fileSystem.IFree(node->iNodeNo);
        }
        node->IUpdate((int)time(NULL));
    }
    node->usingCount--;
}

void INodeManager::UpdateINodeTable()
{
    for (int i = 0; i < INODE_NUM; ++i)
		if(iNodeTable[i].dirLinkNum) 
            iNodeTable[i].IUpdate((int)time(NULL));
}

bool INodeManager::IsLoaded(int no)
{
    if(iNodeMap.find(no) != iNodeMap.end()) return true;
    return false;
}

INode *INodeManager::GetFreeINode()
{
    for(int i = 0; i < INODE_NUM; i++)
        if(iNodeTable[i].usingCount == 0) return &iNodeTable[i];
    return nullptr;
}

void INodeManager::Format()
{
    for(int i = 0; i < INODE_NUM; i++)
        iNodeTable[i].Clean();
}

INodeManager iNodeManager;

IOParameter::IOParameter(){

}

IOParameter::IOParameter(unsigned char *base, int offset, int count){
    this->base = base;
    this->offset = offset;
    this->count = count;
}
