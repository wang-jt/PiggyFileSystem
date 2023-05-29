# 指定要编译的程序名称
TARGET = main

# 指定源文件目录和目标文件目录
SRCDIR = src
OBJDIR = obj

# 指定C++编译器和编译选项
CXX = "C:\Programs\MinGW\bin\g++.exe"
CXXFLAGS =

# 列出所有的头文件
HEADERS = $(wildcard $(SRCDIR)/*.h)

# 查找源文件并生成目标文件列表
SRC = $(wildcard $(SRCDIR)/*.cpp)
OBJ = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRC))

# 默认生成目标文件和可执行文件
all: $(OBJ) $(TARGET)

# 生成可执行文件
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# 生成目标文件
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS) | $(OBJDIR)
	$(CXX) $(CXXFLAGS)-c -o $@ $<

# 创建目标文件目录
$(OBJDIR):
	mkdir $(OBJDIR)

# 清除生成的目标文件和可执行文件
clean:
	del /Q $(OBJDIR)\*.o 

.PHONY: all clean