# 使用的编译器
CC = C:/msys64/ucrt64/bin/g++.exe

# 公共编译选项
CFLAGS = -g -fdiagnostics-color=always

# 需要链接的库
LIBS = -lfreeglut -lopengl32 -lglu32

# 目标程序名
TARGET = main.exe

# 源文件
SRCS = main.c draw.c color.c

# 对应的目标文件
OBJS = $(SRCS:.c=.o)

# 默认目标：编译出 main.exe
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

# 规则：把 .c 编译成 .o
%.o: %.c draw.h color.h
	$(CC) $(CFLAGS) -c $< -o $@

# 清理中间文件
clean:
	del /Q $(OBJS) $(TARGET) 2>nul || true
