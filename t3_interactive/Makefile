# 使用的编译器
CC = C:/msys64/ucrt64/bin/g++.exe

# 公共编译选项
CFLAGS = -g -fdiagnostics-color=always

# 需要链接的库
LIBS = -lfreeglut -lopengl32 -lglu32

# 目标程序名
TARGET = main.exe

# 目标文件
OBJS = main.o draw.o color.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

main.o: main.cpp draw.h color.h
	$(CC) $(CFLAGS) -c main.cpp -o main.o

draw.o: draw.c draw.h color.h
	$(CC) $(CFLAGS) -c draw.c -o draw.o

color.o: color.c color.h
	$(CC) $(CFLAGS) -c color.c -o color.o

clean:
	del /Q $(OBJS) $(TARGET) 2>nul || true
