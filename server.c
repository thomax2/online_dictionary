#include "server.h"

void cli_inf_print(List PtrL)
{
    List p = PtrL;
    if(p == NULL){
        printf("no user log\n");
        return;
    }
    if(p->next == NULL)
        printf("no user log");
    while(p->next != NULL){
        printf("user:%s \t", (p->next)->id);
        p = p->next;
    }
    printf("\n");
    return;
}

void* cli_inf_loop_print(void *arg)
{
    pthread_detach(pthread_self());
    List PtrL = (List)arg;
    while(1)
    {
        cli_inf_print(PtrL); 
        sleep(3);
    }
    pthread_exit(NULL);
}

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
    
    List PtrL = list_create();
    pthread_t tid;
    
    if(pthread_create(&tid,NULL,cli_inf_loop_print,PtrL) != 0)
    {
        printf("thread_create fail\n");
        return -1;
    }

    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);
    msg cli_msg; 
    while(1){
        aID = accept(sockfd,(sockaddr*)&client_addr,&client_addr_len);
        if(aID != -1){
            printf("listen new client %d \n",aID);
        }
        cli_msg.newfd = aID;
        cli_msg.L = PtrL;
        if(pthread_create(&tid, NULL, rcv_cli_proc, &cli_msg) != 0){
			printf("pthread_create\n");
			return -1;
        }
    }
    close(aID);
    return 0;
}
