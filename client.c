#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "sqlite3.h"

typedef struct sockaddr sockaddr;
typedef struct user_msg{
    char type;
    char ID[20];
    char password[20];
}user_msg;
// use to test version control
int main(int argc, const char *argv[])
{
    user_msg usermsg;
    int sockcd = socket(AF_INET,SOCK_STREAM,0);
    if(sockcd != -1)
    {
        printf("client build socket sucess\n");
    }
    
    
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET; // respond with sockfd para
    serv_addr.sin_port = htons(8000); // host Byte order trans to net Byte order
    serv_addr.sin_addr.s_addr = inet_addr("10.0.2.15"); //string to ip addr
    int r;

    r = connect(sockcd,(sockaddr *)&serv_addr,sizeof(serv_addr)); //127...is host addr connect not block
    if(r != -1)
    {
        printf("connect sucess\n");
    }   
    else
        printf("connect fail\n");

    char ch_temp;
    int states = 0;//0 user uncertain; 1 user certain;
    int len;
    int Recv_len;
    char recBuf[200];
    char sendBuf[100];
    while(1)
    {
        if(states == 0)
        {
            printf("input R: Register\tL: Log in\tQ: quit\n");
            usermsg.type =  getchar();
            while ((ch_temp = getchar()) != '\n' && ch_temp != EOF);
            if(usermsg.type == 'R'){
                printf("Register ID: ");  
                fgets(usermsg.ID,sizeof(usermsg.ID),stdin);
                len = strlen(usermsg.ID);
                if(len > 0 && usermsg.ID[len-1] == '\n')
                    usermsg.ID[len-1] = '\0';
                printf("Register Password: ");
                fgets(usermsg.password,sizeof(usermsg.password),stdin);
                len = strlen(usermsg.password);
                if(len > 0 && usermsg.password[len-1] == '\n')
                    usermsg.password[len-1] = '\0';
            }
            else if(usermsg.type == 'Q'){
                break;
            }
            else if(usermsg.type == 'L'){
                printf("ID: ");  
                fgets(usermsg.ID,sizeof(usermsg.ID),stdin);
                len = strlen(usermsg.ID);
                if(len > 0 && usermsg.ID[len-1] == '\n')
                    usermsg.ID[len-1] = '\0';
                printf("Password: ");
                fgets(usermsg.password,sizeof(usermsg.password),stdin);
                len = strlen(usermsg.password);
                if(len > 0 && usermsg.password[len-1] == '\n')
                    usermsg.password[len-1] = '\0';
            }
            send(sockcd,&usermsg,sizeof(usermsg),0);
            
            Recv_len = recv(sockcd,recBuf,200,0);
//            len = strlen(recBuf);
//            if(len > 0 && recBuf[len-1] == '\n')
//                    recBuf[len-1] = '\0';
            if(strcmp(recBuf,"Log in success\n") == 0)
                states = 1;
            if(Recv_len != -1)
                printf("Server:%s\n",recBuf);
        }
        else if(states == 1)
        {
            printf("input S: search word\tE: Exit account\tQ: quit\n");
            usermsg.type =  getchar();
            while ((ch_temp = getchar()) != '\n' && ch_temp != EOF);
            if(usermsg.type == 'E')
            {
                states = 0;
                send(sockcd,&usermsg,sizeof(usermsg),0);
            }else if(usermsg.type == 'Q'){
                break;
            }else if(usermsg.type == 'S'){
                printf("word: ");  
                fgets(usermsg.ID,sizeof(usermsg.ID),stdin);
                len = strlen(usermsg.ID);
                if(len > 0 && usermsg.ID[len-1] == '\n')
                    usermsg.ID[len-1] = '\0';
                send(sockcd,&usermsg,sizeof(usermsg),0);
            }
            Recv_len = recv(sockcd,recBuf,200,0);
            if(Recv_len != -1)
                printf("Server:%s\n",recBuf);
        }
    }
    close(sockcd);
    return 0;
}
