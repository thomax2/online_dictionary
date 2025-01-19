#include "server.h"

int main(int argc, const char *argv[])
{
    int sockfd = socket(AF_INET,SOCK_STREAM,0); // build socket
    if(sockfd != -1)
    {
        printf("build socket sucess\n"); 
    }
    
    int aID;
    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET; // respond with sockfd para
    serv_addr.sin_port = htons(8000); // host Byte order trans to net Byte order
    serv_addr.sin_addr.s_addr = inet_addr("10.0.2.15"); //string to ip addr
    bind(sockfd,(sockaddr*)&serv_addr,sizeof(serv_addr)); //将套接字绑定到本地地址和端口上。
    
    listen(sockfd,5);
    printf("waiting client\n");

    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);
    pthread_t tid;
    while(1)
    {
        aID = accept(sockfd,(sockaddr*)&client_addr,&client_addr_len);
        if(aID != -1){
            printf("listen new client\n");
        }
        
        if(pthread_create(&tid, NULL, rcv_cli_proc, &aID) != 0)
		{
			printf("pthread_create\n");
			return -1;
        }
    }

    close(aID);
    return 0;
}
