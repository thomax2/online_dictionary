#include "server.h"

int main(int argc, const char *argv[])
{
    int sockfd = socket(AF_INET,SOCK_STREAM,0); // build socket
    if(sockfd != -1)
    {
        printf("build socket sucess\n"); 
    }

    sqlite3* userdb = NULL;
    if (sqlite3_open("./user.db",&userdb) != SQLITE_OK) {
        printf("user_db open fail\n");
    }
    
    sqlite3* hisdb = NULL;
    if (sqlite3_open("./history.db",&hisdb) != SQLITE_OK) {
        printf("history_db open fail\n");
    }

    sqlite3* dictdb = NULL;
    if (sqlite3_open("./englishwords.db",&dictdb) != SQLITE_OK) {
        printf("englishwords_db open fail\n");
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
    aID = accept(sockfd,(sockaddr*)&client_addr,&client_addr_len);
    
    if(aID != -1){
        printf("listen new client\n");
    }
    
    int states =0; //0 user uncertain; 1 user certain;
    user_msg usermsg;
    user_msg usermsg_c;
    int Recv_len;
    char recBuf[200]; // receive data from client
    while(1)
    {
        Recv_len = recv(aID,&usermsg,sizeof(usermsg),0); //receive data from client
        printf("%d\n",Recv_len);
        if(Recv_len > 0)
            printf("%s\t%s\n",usermsg.ID,usermsg.password);
        else
            break;
        if(states == 0){ 
            if(usermsg.type == 'R'){
                states = user_register(userdb, usermsg, aID);
            }
            else if(usermsg.type == 'L'){
                states = user_log(userdb,hisdb,usermsg,aID,&usermsg_c);
            }
        }
        else if(states == 1){
            if(usermsg.type == 'E'){
                states = user_exit(aID);                
            }
            else if(usermsg.type == 'S'){
                states = word_search(dictdb,hisdb,usermsg,usermsg_c,aID);
            }
        }
    }
    close(aID);
    sqlite3_close(userdb);
    sqlite3_close(hisdb);
    sqlite3_close(dictdb);
    return 0;
}
