#ifndef BUFFER_H
#define BUFFER_H
#include <cstdio>
const int BUF_SIZE = 512, BUF_NUM = 32;

class Buffer {
public:
    enum BFlag {
        B_DONE = 0x1,
        B_DELWRI = 0x2
    };
    int flag, blkno, wcount;
    unsigned char* addr;   
    Buffer* prev, *next;
public:
    Buffer();
    void Pick();
    void Insert(Buffer* p, Buffer* n);
    bool HasFlag(BFlag f);
    void SetFlag(BFlag f);
    void UnsetFlag(BFlag f);
    void CleanFlag();
    friend class BufferManager;
};

class DeviceDriver{
public:
    FILE *fp;
public:
    DeviceDriver();
    ~DeviceDriver();
    void Read(unsigned char* addr, int size, int offset = 0);
    void Write(unsigned char* addr, int size, int offset = 0);
    void Open(bool format = false);
    void Format();
};

class BufferManager{

public:
    unsigned char addr[BUF_NUM][BUF_SIZE];
    Buffer buffer[BUF_NUM];
    Buffer *lru;
    Buffer lruitem;
public:
    BufferManager();
    Buffer *GetBlk(int blkno);
    Buffer *BRead(int blkno);
    void BWrite(Buffer* bp);
    void BDWrite(Buffer* bp);
    void BClear(Buffer* bp);
    void BFlush();
};

extern BufferManager bufferManager;
extern DeviceDriver deviceDriver;
#endif
