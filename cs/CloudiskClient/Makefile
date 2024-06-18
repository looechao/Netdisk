# 编译器
CC = gcc

# 编译器标志
CFLAGS = -Wall -pthread -g

# 链接库
LDFLAGS = -lcrypt

# 查找当前目录下的所有.c文件
SRCS = $(wildcard *.c)

# 可执行文件
TARGET = main

# 默认目标
all: $(TARGET)

# 生成可执行文件
$(TARGET):
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

# 清理生成的文件
clean:
	rm -f $(TARGET)
