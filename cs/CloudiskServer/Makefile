# 编译器
CC = gcc

# 编译器标志
CFLAGS = -Wall -pthread -g

# 链接器标志
LDFLAGS = -lcrypto -lcrypt 
LIBS = -L /usr/lib64/mysql -lmysqlclient

# 目标可执行文件
TARGET = main

# 获取所有的 .c 文件
SRCS = $(wildcard *.c)

# 规则
all:clean $(TARGET)

# 链接可执行文件
$(TARGET):
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS) $(LIBS)

# 清理
clean:
	@if [ -f $(TARGET) ]; then rm -f $(TARGET); fi

.PHONY: all clean

