#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "INode.h"
#include "Buffer.h"

class SuperBlock {
public:
    const static int MAX_NBLOCK = 100;
    const static int MAX_NINODE = 100;

public:
    int	freeBlockNum;
    int	freeBlockList[MAX_NBLOCK];
    int	freeDiskINodeNum;
    int	freeDiskINodeList[MAX_NINODE];
    int superBlockUpdatedFlag, lastUpdatedTime;
    int	padding[256 - MAX_NBLOCK - MAX_NINODE - 4];

public:
    void setUpdated(bool flag = true);
    bool getUpdated();
};

class FileSystem{
public:
    static const int BLOCK_SIZE = 512;
    static const int DISK_SIZE = 1024 * 2 * 16; // 16MB
    static const int SUPERBLOCK_START_SECTOR = 0;
    static const int INODE_ZONE_START_SECTOR = 2; 
    static const int INODE_ZONE_SIZE = 1022;
    static const int MAX_INODE_NUMBERS_IN_INODE_ZONE = INODE_ZONE_SIZE * 8;
    static const int DATA_ZONE_START_SECTOR = INODE_ZONE_START_SECTOR + INODE_ZONE_SIZE;
    static const int DATA_ZONE_END_SECTOR = DISK_SIZE - 1;
    static const int DATA_ZONE_SIZE = DISK_SIZE - DATA_ZONE_START_SECTOR;
public:
    SuperBlock* superBlock;

public:
    FileSystem();
    ~FileSystem();
    void FormatSuperBlock();
    void FormatFileSystem();
    void LoadSuperBlock();
    void Update();//更新 SuperBlock
    INode* IAlloc();
    void IFree(int number); //INode
    Buffer* Alloc();
    void Free(int blkno);
};

extern FileSystem fileSystem;
#endif