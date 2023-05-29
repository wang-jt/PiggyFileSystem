# PigFileSystem
同济大学操作系统课程设计(CS100436 01, 23Spring). 采用C++实现的二级文件系统

### 使用说明

在当前目录下执行make生成exe
直接打开exe程序即可进入二级文件系统。
支持以下命令：
1)ls ：显示当前文件夹下的全部文件和文件夹
2)mkdir <dirname> ：在当前文件夹下创建目录<dirname>
3)rmdir <dirname> ：在当前文件夹下删除目录<dirname>
4)ffmormat ：格式化文件系统
5)exit ：退出系统（必须使用该命令退出系统，否则会导致缓存未写入磁盘而产生错误！）
6)fcreat <filename> <mode> ：创建名为 <filename> 的文件，其读写权限为<mode>（mode可以为r，w或rw）
7)foepn <filename> <mode> ：打开名为 <filename> 的文件，打开方式为<mode>（mode可以为r，w或rw），其会返回该文件的文件操作符fd
8)fread <fd> <size> ：从文件描述符<fd>指向的文件中读取最大<size>个字节到缓存区，如果<size>为0则默认将文件读取完毕，输出实际读取的字节数
9)fwrite <fd> <size> ：从缓存区写入最大<size>个字节到文件描述符<fd>指向的文件中，如果<size>为0则默认将缓存区全部写入，输出实际写入的字节数
10)fseek <fd> <offset> <mode>：将文件描述符<fd>指向的文件偏移到<mode>和<offset>指向的位置，其中mode可以取0、1、2，其含义分别为从文件头、当前位置、文件尾开始计算偏移量。
此外，为了与一级文件系统交互的方便，我还新增了三个命令：
11)fin <filename>：将<filename>指向的一级操作系统的实际文件读取到缓存区中
12)fout <filename>：将缓存区中的所有字节写入<filename>指向的一级操作系统的实际文件
13)bshow ：查看缓存区信息
