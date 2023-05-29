#ifndef INODE_H
#define INODE_H
#include "Buffer.h"
#include <map>

struct IOParameter
{
    unsigned char* base;
    int offset;
    int count;
    IOParameter();
    IOParameter(unsigned char* base, int offset, int count);
};

class DiskINode { // 64字节
public:
    int mode, dirLinkNum;
    short userMode, __none__;
    int	fileSize, addr[10];
    int	lastViewTime, lastEditTime;
public:
    DiskINode();
    ~DiskINode();
};

enum INodeFlag {
    IFREE = 0x0,
    IFILE = 0x1,
    IUPD = 0x2,		// 修改过，需要更新相应外存INode
    IACC = 0x4,    // 访问过，需要修改最近一次访问时间
    IDIR = 0x8,
};

class INode{
public:
    int     mode, flag;
    int     usingCount, dirLinkNum;
    int	    lastViewTime, lastEditTime, iNodeNo;
    short	userMode, __none__;
    int		fileSize, addr[10];
public:
    INode();
    ~INode();
    int IRead(IOParameter para);
    int IWrite(IOParameter para);
    int BMap(int lbn);
    void IUpdate(int time);
    void ITrunc();//释放Inode
    void Clean();
    void ICopy(Buffer* bp, int iNumber);
private:
    DiskINode WriteDiskINode();
    void readDIskINode(DiskINode diskINode);
public:
    enum UserModeFlag{READ=0x1, WRITE=0x2};
    void SetFileUserMode(int flag);
    bool HasFileUserMode(int flag);
};

class INodeManager{
    public:
    static const int INODE_NUM = 128;

private:
    std::map<int, INode*> iNodeMap;
    INode iNodeTable[INODE_NUM];
public:
    INodeManager();
    ~INodeManager();
    INode* IGet(int no);
    void IPut(INode* node);
    void UpdateINodeTable();
    bool IsLoaded(int no);
    INode* GetFreeINode();
    void Format();
};
extern INodeManager iNodeManager;
#endif