#include "File.h"
#include <iostream>
#include <cstring>
#include "Debug.h"

using namespace std;

OpenFileManager openFileManager;
FileManager fileManager;

OpenFileManager::OpenFileManager()
{
    
}

OpenFileManager::~OpenFileManager()
{
}

int OpenFileManager::Alloc(INode *i)
{
    File *f = AllocFile(i);
    return f - openFileTable;
}

void OpenFileManager::Free(int fd)
{
    FreeFile(&(openFileTable[fd]));
}

File *OpenFileManager::GetFile(int fd)
{
    Debug(7, "GetFile: fd = %d flag=%d\n", fd, openFileTable[fd].flag);
    if(fd >= 0 && fd < MAX_FILES && !openFileTable[fd].IsEmpty())
        return &(openFileTable[fd]);
    return nullptr;
}

void OpenFileManager::init(){
    for(int i = 0; i < MAX_FILES; i++)
        openFileTable[i].Clean();
}

File *OpenFileManager::AllocFile(INode *inode)
{
    for(int i = 0; i < MAX_FILES; i++)
        if(openFileTable[i].IsEmpty()){
            openFileTable[i].SetINode(inode);
            return &(openFileTable[i]);
        }
            
}

void OpenFileManager::FreeFile(File *f)
{
    f->Clean();
}

FileManager::FileManager(){
    curDirINode = nullptr;
}

FileManager::~FileManager(){
    iNodeManager.IPut(curDirINode);
}

void FileManager::Ls(){
    vector<DirEntry> dirList = ((DirINode*)curDirINode)->DirList();
    for(int i = 2; i < dirList.size(); i++){
        cout << dirList[i].name << " ";
    }
    cout << endl;
}

void FileManager::init(){
    if(curDirINode != nullptr)
        iNodeManager.IPut(curDirINode);
    curDirINode = iNodeManager.IGet(0);
}

static string GetDir(vector<string> dir){
    string s = "/";
    for(int i = 0; i < dir.size(); i++)
        s += dir[i] + (i == dir.size() - 1 ? "" : "/");
    return s;
}
static vector<string> cdlist;
string FileManager::GetCurDir(){
    return GetDir(curDirStringList);
}


bool FileManager::Cd(string dirName)
{
    auto i = NameI(dirName, DirectorySearchMode::D_CD);
    if(i == nullptr) return false;
    iNodeManager.IPut(curDirINode);
    curDirINode = i;
    curDirStringList = cdlist;
    return true;
}

bool CheckName(string name){
    if(name.length() > 24){
        cout << "Name length > 24, is too long." << endl;
        return false;
    } else if(name.length() == 0){
        cout << "Name cannot be empty." << endl;
        return false;
    } else if(name.find('/') != name.npos || name.find('\\') != name.npos || name.find(' ') != name.npos){
        cout << "Name cannot contain '/', '\\', ' '." << endl;
        return false;
    } return true;
}

bool FileManager::Mkdir(string dirName)
{
    if(!CheckName(dirName)) return false;
    if(((DirINode*)curDirINode)->FindNode(dirName).iNodeIndex != -1){
        cout << "Directory already exists." << endl;
        return false;
    }
    auto c = fileSystem.IAlloc();
    if(c->iNodeNo == -1) return false;
    ((DirINode*)c)->InitDirINode(curDirINode->iNodeNo);
    int iNodeIndex = c->iNodeNo;
    ((DirINode*)curDirINode)->AddNode(iNodeIndex, INodeFlag::IDIR, dirName);
    iNodeManager.IPut(c);
    return true;
}

bool FileManager::Rmdir(string dirName, INode* base)
{
    if(base == nullptr) base = curDirINode;
    if(!CheckName(dirName)) return false;
    auto dire = ((DirINode*)base)->FindNode(dirName, INodeFlag::IDIR);
    if(dire.iNodeIndex == -1){
        cout << "Directory not found." << endl;
        return false;
    }
    auto c = iNodeManager.IGet(dire.iNodeIndex);
    ((DirINode*)c)->DeleteAllNode();
    iNodeManager.IPut(c);
    ((DirINode*)base)->DeleteNode(dirName, INodeFlag::IDIR);
    return true;
}

int FileManager::Create(string fileName, string mode){
    if(!CheckName(fileName)) return -1;
    if(((DirINode*)curDirINode)->FindNode(fileName, INodeFlag::IFILE).iNodeIndex != -1){
        cout << "File already exists." << endl;
        return -1;
    }
    auto c = fileSystem.IAlloc();
    if(c->iNodeNo == -1) return -1;
    c->SetFileUserMode((mode.find('r') != mode.npos ? INode::UserModeFlag::READ : 0) | (mode.find('w') != mode.npos ? INode::UserModeFlag::WRITE : 0));
    Debug(6, "FileManager::Create cflag = %d, mode = %s, flag=%d\n", c->userMode, mode.c_str(),c->flag);
    ((DirINode*)curDirINode)->AddNode(c->iNodeNo, INodeFlag::IFILE, fileName);
    iNodeManager.IPut(c);
    return 0;
}

int FileManager::Delete(string fileName)
{
    if(!CheckName(fileName)) return -1;
    auto dire = ((DirINode*)curDirINode)->FindNode(fileName, INodeFlag::IFILE);
    if(dire.iNodeIndex == -1){
        cout << "File not found." << endl;
        return -1;
    }
    ((DirINode*)curDirINode)->DeleteNode(fileName, INodeFlag::IFILE);
    return 0;
}

int FileManager::Open(string fileName, string mode)
{
    int tflag = 0;
    if(!CheckName(fileName)) return -1;
    auto i = NameI(fileName, DirectorySearchMode::D_OPEN);
    if(i == nullptr){
        cout << "File not found." << endl;
        return -1;
    }
    auto c = i;
    Debug(6, "cflag = %d, mode = %s\n", c->userMode, mode.c_str());
    if(mode.find('r') != mode.npos && !c->HasFileUserMode(INode::UserModeFlag::READ)){
        cout << "File is not readable." << endl;
        iNodeManager.IPut(c);
        return -1;
    }
    if(mode.find('w') != mode.npos && !c->HasFileUserMode(INode::UserModeFlag::WRITE)){
        cout << "File is not writable." << endl;
        iNodeManager.IPut(c);
        return -1;
    }
    int ret = openFileManager.Alloc(c);
    openFileManager.openFileTable[ret].SetFlag((mode.find('r') != mode.npos ? File::FREAD : 0 ) | (mode.find('w') != mode.npos ? File::FWRITE : 0));
    return ret;
}

int FileManager::Close(int fd)
{
    if(openFileManager.GetFile(fd) == nullptr){
        cout << "fd <"<<fd<<"> is not opened." << endl;
        return -1;
    }
    openFileManager.Free(fd);
    return 0;
}

int FileManager::Seek(int fd, int offset, int origin)
{
    File *f = openFileManager.GetFile(fd);
    if(f == nullptr){
        cout << "fd <"<<fd<<"> is not opened." << endl;
        return -1;
    }
    auto i = f->inode;
    if(origin == SEEK_SET){
        if(offset < 0 || offset > i->fileSize){
            cout << "Invalid offset." << endl;
            return -1;
        }
        f->offset = offset;
    } else if(origin == SEEK_CUR){
        if(f->offset + offset < 0 || f->offset + offset > i->fileSize){
            cout << "Invalid offset." << endl;
            return -1;
        }
        f->offset += offset;
    } else if(origin == SEEK_END){
        if(offset > 0 || offset < -i->fileSize){
            cout << "Invalid offset." << endl;
            return -1;
        }
        f->offset = i->fileSize + offset;
    } else {
        cout << "Invalid origin." << endl;
        return -1;
    }
    cout << "Seek success. offset = "<< offset << endl;
    return 0;
}

int FileManager::Rdwr(File::FileFlags mode, int fd, unsigned char *ptr, int size)
{
    File *f = openFileManager.GetFile(fd);
    if(f == nullptr){
        cout << "fd <"<<f<<"> is not opened." << endl;
        return -1;
    }
    if(mode == File::FileFlags::FWRITE){
        if(!f->HasFlag(File::FileFlags::FWRITE)){
            cout << "File is not writable." << endl;
            return -1;
        } else {
            int writesize = f->inode->IWrite(IOParameter(ptr, f->offset, size));
            f->offset += writesize;
            return writesize;
        }
    } else if(mode == File::FileFlags::FREAD){
        if(!f->HasFlag(File::FileFlags::FREAD)){
            cout << "File is not readable." << endl;
            return -1;
        } else {
            int readsize = f->inode->IRead(IOParameter(ptr, f->offset, size));
            f->offset += readsize;
            return readsize;
        }
    }
}

INode *FileManager::NameI(string name, DirectorySearchMode mode)
{
    int p;
    INode* cnode = iNodeManager.IGet(curDirINode->iNodeNo);
    cdlist = curDirStringList;
    do {
        if((p = name.find('/')) == 0){ //从根目录开始
            iNodeManager.IPut(cnode);
            cnode = iNodeManager.IGet(0);
        } else {
            string fname = p == name.npos ? name : name.substr(0, p);
            int pt = ((DirINode*)cnode)->FindNode(fname, p == name.npos ? (mode == DirectorySearchMode::D_CD ? INodeFlag::IDIR : INodeFlag::IFILE) : INodeFlag::IDIR).iNodeIndex;
            if(pt == -1){
                if(mode == DirectorySearchMode::D_CREATE && p == name.npos) return cnode;
                cout << (mode == DirectorySearchMode::D_CD ? "No such directory:<" : "No such file :<") << fname << ">." << endl;
                iNodeManager.IPut(cnode); return nullptr; 
            }
            iNodeManager.IPut(cnode);
            cnode = iNodeManager.IGet(pt);
            Debug(5, "NameI fname = %s, pt = %d, cnode->iNodeNo = %d\n", fname.c_str(), pt, cnode->iNodeNo);
            if(mode == DirectorySearchMode::D_CD)
                if(fname == "..")
                    cdlist.pop_back();
                else if(fname != ".")
                    cdlist.push_back(fname);
        }
        name = name.substr(p + 1);
    } while (p != name.npos);
    return cnode;
}

DirEntry DirINode::FindNode(string name, int mode)
{
    int dirSize = fileSize / sizeof(DirEntry);
    DirEntry *dirEntryList = new DirEntry[dirSize];
    IRead(IOParameter((unsigned char *)dirEntryList, 0, fileSize));
    for(int i = 0; i < dirSize; i++){
        if(strcmp(dirEntryList[i].name, name.c_str()) == 0 && mode == dirEntryList[i].mode){
            DirEntry ret = dirEntryList[i];
            delete[] dirEntryList;
            return ret;
        }
    }
    delete[] dirEntryList;
    return DirEntry(-1, -1, "");
}

bool DirINode::AddNode(int iNodeIndex, int mode, string name)
{
    if(FindNode(name, mode).iNodeIndex != -1) return false;
    auto t = DirEntry(iNodeIndex, mode, name);
    Debug(6, "DirINode::AddNode t.name = %s, t.iNodeIndex = %d, t.mode = %d\n", t.name, t.iNodeIndex, t.mode);
    IWrite(IOParameter((unsigned char *)&t, fileSize, sizeof(DirEntry)));
    auto c = iNodeManager.IGet(iNodeIndex);
    c->dirLinkNum++;
    iNodeManager.IPut(c);
    return true;
}

bool DirINode::DeleteNode(string name, int mode)
{
    int dirSize = fileSize / sizeof(DirEntry);
    DirEntry *dirEntryList = new DirEntry[dirSize];
    IRead(IOParameter((unsigned char *)dirEntryList, 0, fileSize));
    for(int i = 0; i < dirSize; i++){
        if(strcmp(dirEntryList[i].name, name.c_str()) == 0 && mode == dirEntryList[i].mode){
            auto c = iNodeManager.IGet(dirEntryList[i].iNodeIndex);
            c->dirLinkNum--;
            iNodeManager.IPut(c);
            for(int j = i + 1; j < dirSize; j++){
                dirEntryList[j - 1] = dirEntryList[j];
            }
            IWrite(IOParameter((unsigned char *)dirEntryList, 0, fileSize - sizeof(DirEntry)));
            fileSize -= sizeof(DirEntry);
            delete[] dirEntryList;
            return true;
        }
    }
    delete[] dirEntryList;
    return false;
}

bool DirINode::DeleteAllNode()
{
    int dirSize = fileSize / sizeof(DirEntry);
    DirEntry *dirEntryList = new DirEntry[dirSize];
    IRead(IOParameter((unsigned char *)dirEntryList, 0, fileSize));
    for(int i = 0; i < dirSize; i++){
        auto c = iNodeManager.IGet(dirEntryList[i].iNodeIndex);
        c->dirLinkNum--;
        iNodeManager.IPut(c);
    }
    delete[] dirEntryList;
    return true;
}

void DirINode::InitDirINode(int parent)
{
    fileSize = 0;
    this->flag |= INodeFlag::IDIR;
    AddNode(this->iNodeNo, INodeFlag::IDIR, ".");
    AddNode(parent, INodeFlag::IDIR, "..");
}

vector<DirEntry> DirINode::DirList()
{
    Debug(6, "DirList::Filesize = %d\n", fileSize);
    int dirSize = fileSize / sizeof(DirEntry);
    DirEntry *dirEntryList = new DirEntry[dirSize];
    IRead(IOParameter((unsigned char *)dirEntryList, 0, fileSize));
    vector<DirEntry> ret;
    for(int i = 0; i < dirSize; i++){
        ret.push_back(dirEntryList[i]);
    }
    delete[] dirEntryList;
    return ret;
}

DirEntry::DirEntry(){
    
}

DirEntry::DirEntry(int iNodeIndex, int mode, string name){
    this->iNodeIndex = iNodeIndex, this->mode = mode;
    strcpy(this->name, name.c_str());
}

bool File::IsEmpty()
{
    return flag == 0;
}

bool File::HasFlag(FileFlags f)
{
    return flag & f;
}

void File::SetFlag(int f)
{
    Debug(6, "File::SetFlag f = %d\n", f);
    flag |= f;
}

void File::UnsetFlag(int f){
    flag &= ~f;
}

void File::Clean(){
    flag = FEMPTY;
    offset = 0;
    if(inode != nullptr) iNodeManager.IPut(inode);
    inode = nullptr;
}

void File::SetINode(INode *i){
    inode = i;
}

int FileManager::Write(int fd, unsigned char *ptr, int size)
{
    return Rdwr(File::FileFlags::FWRITE, fd, ptr, size);
}

int FileManager::Read(int fd, unsigned char *ptr, int size)
{
    return Rdwr(File::FileFlags::FREAD, fd, ptr, size);
}

int FileManager::Size(int fd)
{
    if(openFileManager.GetFile(fd) == nullptr) return -1;
    return openFileManager.GetFile(fd)->inode->fileSize;
}
