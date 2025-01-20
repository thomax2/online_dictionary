# 定义编译器
CC=gcc

# 定义目标文件
SERVER=server
CLIENT=client
LDFLAGS = -lsqlite3

# 定义源文件
SERVER_SRC=server.c
CLIENT_SRC=client.c
SER_HEAD=server.h list.h
CLI_HEAD=client.h

# 默认目标
all: $(SERVER) $(CLIENT)

# 编译服务器
$(SERVER):$(SERVER_SRC) $(SER_HEAD)
	$(CC) $(SERVER_SRC) -o $(SERVER) $(LDFLAGS) -lpthread 

# 编译客户端
$(CLIENT):$(CLIENT_SRC) $(CLI_HEAD)
	$(CC) $(CLIENT_SRC) -o $(CLIENT) $(LDFLAGS) -lpthread

# 清理目标
clean:
	rm -f $(SERVER) $(CLIENT)

# .PHONY 是一个伪目标，确保即使存在名为 all、clean 的文件，也会执行对应的规则
.PHONY: all clean

