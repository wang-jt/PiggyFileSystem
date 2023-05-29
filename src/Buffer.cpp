#include "Buffer.h"
#include "FileSystem.h"
#include "Debug.h"
#include <cstring>
#include <iostream>
using namespace std;

Buffer::Buffer(){
    flag = 0, blkno = 0, wcount = 0;
    addr = nullptr;   
    prev = nullptr, next = nullptr;
}

void Buffer::Pick()
{
    if(prev != nullptr)
        prev->next = next;
    if(next != nullptr)
        next->prev = prev;
    prev = nullptr;
    next = nullptr;
}

void Buffer::Insert(Buffer *p, Buffer *n)
{
    if(p != nullptr)
    {
        prev = p;
        p->next = this;
    }
    if(n != nullptr)
    {
        next = n;
        n->prev = this;
    }
}

bool Buffer::HasFlag(BFlag f)
{
    return flag & f;
}

void Buffer::SetFlag(BFlag f)
{
    if(~HasFlag(f))
        flag = flag | f;
}

void Buffer::UnsetFlag(BFlag f)
{
    if(HasFlag(f))
        flag = flag & (~f);
}

void Buffer::CleanFlag()
{
    flag = 0;
}

BufferManager::BufferManager()
{
    lru = &lruitem;
    for (int i = 0; i < BUF_NUM; i++)
    {
        buffer[i].Insert(i == 0 ? lru : &buffer[i - 1], nullptr);
        buffer[i].addr = addr[i];
    }
}

Buffer *BufferManager::GetBlk(int blkno)
{
    Buffer *ret = nullptr, *end = nullptr;
    for(Buffer* p = lru->next; p != nullptr; p = p->next){ //选中了可复用块
        if(p->blkno == blkno){
            ret = p;
            break;
        }
        if(p->next == nullptr)
            end = p;
    }
    if(ret == nullptr) {
        ret = end;
        if(ret->flag & Buffer::B_DELWRI) //碰到的是脏块
            BWrite(ret);
        ret->CleanFlag();
        
    }
    ret->blkno = blkno;
    ret->Pick();
    ret->Insert(lru, lru->next);
    return ret;
}

Buffer *BufferManager::BRead(int blkno)
{
    Debug(1, "BREAD %d\n", blkno);
    Buffer *bp = GetBlk(blkno);
    if(!bp->HasFlag(Buffer::B_DONE)){
        deviceDriver.Read(bp->addr, BUF_SIZE, bp->blkno * BUF_SIZE);
        bp->SetFlag(Buffer::B_DONE);
    }
    return bp;
}

void BufferManager::BWrite(Buffer *bp)
{
    Debug(1, "BWRITE %d\n", bp->blkno);
    //DebugHex(1, bp->addr, 128);
    deviceDriver.Write(bp->addr, BUF_SIZE, bp->blkno * BUF_SIZE);
    bp->UnsetFlag(Buffer::B_DELWRI);
    bp->SetFlag(Buffer::B_DONE);
}

void BufferManager::BDWrite(Buffer *bp)
{
    bp->SetFlag(Buffer::B_DELWRI);
}

void BufferManager::BClear(Buffer *bp)
{
    memset(bp->addr, 0, BUF_SIZE);
    bp->SetFlag(Buffer::B_DELWRI);
}

void BufferManager::BFlush()
{
    for(int i = 0; i < BUF_NUM; i++)
        if(buffer[i].HasFlag(Buffer::B_DELWRI))
            BWrite(&buffer[i]);
}

BufferManager bufferManager;
DeviceDriver deviceDriver;

const char* DISK_NAME = "2050254-disk.img";
DeviceDriver::DeviceDriver()
{
    this->Open();
}

DeviceDriver::~DeviceDriver()
{
    fclose(fp);
}

void DeviceDriver::Read(unsigned char *addr, int size, int offset)
{
    fseek(fp, offset, SEEK_SET);
    fread(addr, size, 1, fp);
}

void DeviceDriver::Write(unsigned char* addr, int size, int offset){
    fseek(fp, offset, SEEK_SET);
    fwrite(addr, size, 1, fp);
}

void DeviceDriver::Format()
{
    fclose(fp);
    Open(true);
}

void DeviceDriver::Open(bool format)
{
    unsigned char *bufa = new unsigned char[FileSystem::BLOCK_SIZE * FileSystem::DISK_SIZE];
    memset(bufa, 0, FileSystem::BLOCK_SIZE * FileSystem::DISK_SIZE);
    if (!format){
        this->fp = fopen(DISK_NAME, "rb");
        if(fp != NULL) fread(bufa, FileSystem::BLOCK_SIZE * FileSystem::DISK_SIZE, 1, fp);
        fclose(fp);
    }
    this->fp = fopen(DISK_NAME, "wb+");
    fwrite(bufa, FileSystem::BLOCK_SIZE * FileSystem::DISK_SIZE, 1, fp);
    fseek(fp, 0, SEEK_SET);
    delete[] bufa;
}
