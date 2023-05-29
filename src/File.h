#ifndef FILE_H
#define FILE_H
#include "INode.h"
#include "FileSystem.h"
#include <string>
#include <vector>
using namespace std;

class File {
public:
	enum FileFlags {
        FEMPTY = 0x0,
		FREAD = 0x1,
		FWRITE = 0x2,
	};
	int flag = 0, offset = 0;
	INode* inode = nullptr;
public:
    bool IsEmpty();
    bool HasFlag(FileFlags f);
    void SetFlag(int f);
    void UnsetFlag(int f);
    void Clean();
    void SetINode(INode* i);
};

class OpenFileManager {
public:
    static const int MAX_FILES = 128;
    File openFileTable[MAX_FILES];
public:
    OpenFileManager();
    ~OpenFileManager();
    int Alloc(INode *i);
    void Free(int fd);
    File* GetFile(int fd);
    void init();
private:
    File* AllocFile(INode *i);
    void FreeFile(File* f);
};
extern OpenFileManager openFileManager;

struct DirEntry{
    static const int DIR_ENTRY_SIZE = 32;
    char name[24] = {0};
    int iNodeIndex = 0, mode = 0;
    DirEntry();
    DirEntry(int iNodeIndex, int mode, string name);
};

class DirINode: public INode {
public:
    DirEntry FindNode(string name, int mode = INodeFlag::IDIR);
    bool AddNode(int iNodeIndex, int mode, string name);
    bool DeleteNode(string name, int mode = INodeFlag::IDIR);
    bool DeleteAllNode();
    void InitDirINode(int parent = 0);
    vector<DirEntry> DirList();
};

class FileManager
{
private:
    vector<string> curDirStringList;

public:
    INode* curDirINode;
public:
    FileManager();
    ~FileManager();
    void init();
public:
    void Ls();
    string GetCurDir();
    bool Cd(string dirName);
    bool Mkdir(string dirName);
    bool Rmdir(string dirName, INode* base = nullptr);
    int Create(string fileName, string mode);
    int Delete(string fileName);
    int Open(string fileName, string mode);
    int Close(int fd);
    int Seek(int fd, int offset, int origin = SEEK_SET);
    int Write(int fd,unsigned char* ptr, int size);
    int Read(int fd, unsigned char* ptr, int size);
    int Size(int fd);
private:
    
    int Rdwr(enum File::FileFlags mode, int fd, unsigned char* ptr, int size);
    enum DirectorySearchMode {D_CD, D_OPEN, D_CREATE, D_DELETE};
    INode* NameI(string name, DirectorySearchMode mode);

    /* 被Creat()系统调用使用，用于为创建新文件分配内核资源 */
    INode* MakNode(unsigned int mode);

    /* 取消文件 */
    void UnLink();
};
extern FileManager fileManager;

#endif