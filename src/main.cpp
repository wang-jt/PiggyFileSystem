#include <bits/stdc++.h>
#include "FileSystem.h"
#include "File.h"
#include "INode.h"
#include "Buffer.h"
#include "Debug.h"
using namespace std;

void init(){
    fileSystem.LoadSuperBlock();
    fileManager.init();
    openFileManager.init();
}

void exit(){
    openFileManager.init();
    iNodeManager.UpdateINodeTable();
    bufferManager.BFlush();
}

unsigned char buff[1024*1024*16];
int buffsize = 0;

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    for(char c : str) 
        if(c == delimiter) {
            tokens.push_back(token);
            token.clear();
        } else token += c;
    if(!token.empty()) tokens.push_back(token);
    return tokens;
}

int main(){
    init();
    string c, t;
    while(true){
        cout << fileManager.GetCurDir() << "$";
        getline(cin, c);
        auto tks = split(c, ' ');
        if(tks[0] == "ls"){
            fileManager.Ls();
        } else if(tks[0] == "cd"){
            if(tks.size() == 1) fileManager.Cd("/");
            else fileManager.Cd(tks[1]);
        } else if(tks[0] == "mkdir"){
            if(tks.size() == 1) cout << "mkdir: missing operand" << endl;
            else fileManager.Mkdir(tks[1]);
        } else if(tks[0] == "rmdir"){
            if(tks.size() == 1) cout << "rmdir: missing operand" << endl;
            else fileManager.Rmdir(tks[1]);
        } else if(tks[0] == "fformat"){
            fileSystem.FormatFileSystem();
        } else if(tks[0] == "fcreat"){
            if(tks.size() < 3) cout << "fcreat: missing operand.\n Usage:fcreate <filename> <mode>" << endl;
            else fileManager.Create(tks[1], tks[2]);
        } else if(tks[0] == "fopen"){
            if(tks.size() < 3) cout << "fopen: missing operand.\n Usage:fopen <filename> <mode>" << endl;
            else cout << "Alloced file descriptor: " << fileManager.Open(tks[1], tks[2]) << endl;
        } else if(tks[0] == "fclose"){
            if(tks.size() < 2) cout << "fclose: missing operand.\n Usage:fclose <fd>" << endl;
            else fileManager.Close(stoi(tks[1]));
        } else if(tks[0] == "fread"){
            if(tks.size() < 3) cout << "fread: missing operand.\n Usage:fread <fd> <size>\n if <size>=0 then set size to bufferSize" << endl;
            else {
                buffsize = fileManager.Read(stoi(tks[1]), buff, (stoi(tks[2]) == 0 ? fileManager.Size(stoi(tks[1])) : stoi(tks[2])));
                cout << "Read " << buffsize << " bytes to buffer." << endl;
            }
        } else if(tks[0] == "fwrite"){
            if(tks.size() < 3) cout << "fwrite: missing operand.\n Usage:fwrite <fd> <size>\n if <size>=0 then set size to bufferSize" << endl;
            else {
                int t = fileManager.Write(stoi(tks[1]), buff, (stoi(tks[2]) == 0 ? buffsize : stoi(tks[2])));
                cout << "Write " << t << " bytes to file." << endl;
            }
        } else if(tks[0] == "flseek"){
            if(tks.size() < 4) cout << "flseek: missing operand.\nUsage:flseek <fd> <offset> <type>\n<type>:\n0=Seek from start\n1=Seek from current\n2=Seek from end" << endl;
            else fileManager.Seek(stoi(tks[1]), stoi(tks[2]), stoi(tks[3]));
        } else if(tks[0] == "fdelete"){
            if(tks.size() < 2) cout << "fdelete: missing operand.\n Usage:fdelete <filename>" << endl;
            else fileManager.Delete(tks[1]);
        } else if(tks[0] == "fin"){
            if(tks.size() < 2) cout << "fin: missing operand.\n Usage:fin <outside file name>" << endl;
            else {
                FILE* a = fopen(tks[1].c_str(), "rb");
                if(a == NULL){cout << "File not found." << endl;}
                else {
                    int t = fread(buff, 1, 1024*1024*16, a);
                    fclose(a);
                    buffsize = t;
                    cout << "Read " << t << " bytes to buffer." << endl;
                }
            }
        } else if(tks[0] == "fout"){
            if(tks.size() < 2) cout << "fout: missing operand.\n Usage:fout <outside file name>" << endl;
            else {
                FILE* a = fopen(tks[1].c_str(), "wb");
                if(a == NULL){cout << "File not found." << endl;}
                else {
                    int t = fwrite(buff, 1, buffsize, a);
                    fclose(a);
                    cout << "Write " << t << " bytes to file." << endl;
                }
            }
        } else if(tks[0] == "bshow"){
            cout << "Buffer size: " << buffsize << endl;
            DebugHex(-1, buff, buffsize <= 512 ? buffsize : 512);
        } else if(tks[0] == "exit"){
            exit();
            break;
        } else cout << "Command not found." << endl;
    }
    
    return 0;
}