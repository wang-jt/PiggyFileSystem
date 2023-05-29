#include "FileSystem.h"
#include "Debug.h"
#include "File.h"
#include <iostream>
#include <cstring>
#include <ctime>
using namespace std;

FileSystem fileSystem;

void SuperBlock::setUpdated(bool flag)
{
    this->superBlockUpdatedFlag = flag;
}

bool SuperBlock::getUpdated()
{
    return this->superBlockUpdatedFlag;
}

FileSystem::FileSystem()
{
    superBlock = new SuperBlock();
}

FileSystem::~FileSystem()
{
    delete superBlock;
}

void FileSystem::FormatSuperBlock()
{
    memset(superBlock, 0, sizeof(SuperBlock));
    superBlock->lastUpdatedTime = (int)time(NULL);
}

void FileSystem::FormatFileSystem()
{
    Debug(4, "Formatting FileSystem...\n");
    this->FormatSuperBlock();
    deviceDriver.Format();
    DiskINode rootDiskINode, emptyDiskINode;
    memset(&rootDiskINode, 0, sizeof(DiskINode));
    rootDiskINode.mode = INodeFlag::IDIR;
    rootDiskINode.dirLinkNum = 1;
    deviceDriver.Write((unsigned char *)&rootDiskINode, sizeof(DiskINode), INODE_ZONE_START_SECTOR);
    for(int i = 1; i < SuperBlock::MAX_NINODE; ++i)
        if(superBlock->freeDiskINodeNum < SuperBlock::MAX_NINODE)
            superBlock->freeDiskINodeList[superBlock->freeDiskINodeNum++] = i;
    for(int i = 0; i < DATA_ZONE_SIZE; i++)
        this->Free(i + DATA_ZONE_START_SECTOR);
    iNodeManager.Format();
    deviceDriver.Write((unsigned char *)superBlock, sizeof(SuperBlock), SUPERBLOCK_START_SECTOR);
    auto i = iNodeManager.IGet(0);
    Debug(5, "Start adding rootINode\n");
    ((DirINode*)i)->AddNode(0, INodeFlag::IDIR, ".");
    ((DirINode*)i)->AddNode(0, INodeFlag::IDIR, "..");
    bufferManager.BFlush();
    Debug(5, "RootINodeFileSize: %d\n", i->fileSize);
    i->IUpdate(time(NULL));
    iNodeManager.IPut(i);
    fileManager.init();
    Debug(4, "Formatting FileSystem finished.\n");
}

void FileSystem::LoadSuperBlock()
{
    deviceDriver.Read((unsigned char *)superBlock, sizeof(SuperBlock), SUPERBLOCK_START_SECTOR);
}

void FileSystem::Update()
{
    if(superBlock->getUpdated()) {
        superBlock->lastUpdatedTime = (int)time(NULL);
        superBlock->setUpdated(false);
        deviceDriver.Write((unsigned char *)superBlock, sizeof(SuperBlock), SUPERBLOCK_START_SECTOR);
    }
    iNodeManager.UpdateINodeTable();
    bufferManager.BFlush();
}

INode *FileSystem::IAlloc()
{
    int no = -1;
    Buffer *buf = nullptr;
    if(superBlock->freeDiskINodeNum == 0){
        int cnt = 0, iNodeNo = -1;
        for (int i = 0; i < INODE_ZONE_SIZE; ++i) {
            buf = bufferManager.BRead(FileSystem::INODE_ZONE_START_SECTOR + i);
            for (int j = 0; j < 8; ++j) {
                int mode = *(int *)(buf->addr + j * sizeof(DiskINode)); //判断是否空闲
                if (mode) continue;
                if(iNodeManager.IsLoaded(++iNodeNo)) continue;
                superBlock->freeDiskINodeList[superBlock->freeDiskINodeNum++] = iNodeNo;
                if(superBlock->freeDiskINodeNum >= SuperBlock::MAX_NINODE) break;
            }
            if(superBlock->freeDiskINodeNum >= SuperBlock::MAX_NINODE) break;
        }
    }
    if(superBlock->freeDiskINodeNum == 0) {
        std::cout << "INode Out of Memory!" << std::endl;
        return nullptr;
    }
    no = superBlock->freeDiskINodeList[--superBlock->freeDiskINodeNum];
    INode* ret = iNodeManager.IGet(no);
    ret->Clean();
    superBlock->setUpdated();
    return ret;
}

void FileSystem::IFree(int number)
{
    if(superBlock->freeDiskINodeNum >= SuperBlock::MAX_NINODE){
        return;
    }
	superBlock->freeDiskINodeList[superBlock->freeDiskINodeNum++] = number;
    superBlock->setUpdated();
}

Buffer *FileSystem::Alloc()
{
    int blkno = superBlock->freeBlockList[--superBlock->freeBlockNum];
    Buffer* buf;
    if(blkno == 0){
        std::cout << "Block Out of Memory!" << std::endl;
        return NULL;
    } 
    if(superBlock->freeBlockNum == 0){ //重新分配
        buf = bufferManager.BRead(blkno);
        int *p = (int *)buf->addr;
        superBlock->freeBlockNum = *p++;
        memcpy(superBlock->freeBlockList, p, sizeof(superBlock->freeBlockList));
    } else buf = bufferManager.GetBlk(blkno);
    if(buf) bufferManager.BClear(buf);
    superBlock->setUpdated();
    return buf;
}

void FileSystem::Free(int blkno)
{
    Buffer* buf;
    if(superBlock->freeBlockNum == SuperBlock::MAX_NBLOCK){
        buf = bufferManager.BRead(blkno);
        memset(buf->addr, 0, FileSystem::BLOCK_SIZE);
        int *p = (int *)buf->addr;
        *p++ = superBlock->freeBlockNum;
        memcpy(p, superBlock->freeBlockList, sizeof(superBlock->freeBlockList));
        superBlock->freeBlockNum = 0;
        bufferManager.BWrite(buf);
    }
    superBlock->freeBlockList[superBlock->freeBlockNum++] = blkno;
    superBlock->setUpdated();
}