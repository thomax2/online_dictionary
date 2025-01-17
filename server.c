#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "sqlite3.h"
#include <time.h>

typedef struct sockaddr sockaddr;
typedef struct user_msg{
    char type;
    char ID[20];
    char password[20];
}user_msg;


static int Insert_Check_Callback(void *data, int argc, char **argv, char **azColName){
    if (argc > 0) {
        int* flag = (int *)data;
        *flag = 1;
    }
    return 0;
}

static int search_Callback(void *data, int argc, char **argv, char ** azColName){
    char* meaning = (char *)data; 
    if (argc >0 ){
        strcpy(meaning,argv[0]);
    }else{
        strcpy(meaning,"NULL!");
    }
    return 0;
}

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
    
    if(aID != -1)
    {
        printf("listen new client\n");
    }
    
    char* errmsg = NULL;
    char* sql_cre_table = "create table if not exists userMsg (id text primary key, password text);"; 
    char sql[200];
    int states =0; //0 user uncertain; 1 user certain;
    int flag_ID = 0;
    int flag_password = 0;
    char sendBuf[200];
    user_msg usermsg;
    user_msg usermsg_c;
    int Recv_len;
    char recBuf[200]; // receive data from client
    char meaning[80];
    char time_str[50];
    time_t current_time;
    struct tm *local_time;
    while(1)
    {
        Recv_len = recv(aID,&usermsg,sizeof(usermsg),0); //receive data from client
        printf("%d\n",Recv_len);
        if(Recv_len > 0)
            printf("%s\t%s\n",usermsg.ID,usermsg.password);
        else
            break;
        if(states == 0){ 
            if(usermsg.type == 'R')
            {
                if (sqlite3_exec(userdb,sql_cre_table,NULL,NULL,&errmsg) != SQLITE_OK) {
                    printf("userdb table create fail\n"); 
                }

                memset(sql, 0, sizeof(sql));
                snprintf(sql,sizeof(sql),"SELECT ID FROM userMsg WHERE ID == '%s' ;",usermsg.ID);
                if(sqlite3_exec(userdb,sql,Insert_Check_Callback,&flag_ID,&errmsg) != SQLITE_OK){
                    printf("Insert Check fail\n"); 
                }
       
                if(flag_ID == 1)
                {
                    flag_ID = 0;
                    memset(sendBuf,0,sizeof(sendBuf));
                    sprintf(sendBuf,"This ID has already been registered\n");
                    send(aID,sendBuf,strlen(sendBuf)+1,0);
                }
                else
                {
                    memset(sendBuf,0,sizeof(sendBuf)); 
                    sprintf(sendBuf,"Account register success\n");
                    send(aID,sendBuf,strlen(sendBuf)+1,0);
                    memset(sql, 0, sizeof(sql));
                    snprintf(sql,sizeof(sql),"INSERT INTO userMsg VALUES('%s', '%s')" \
                            ,usermsg.ID,usermsg.password);
                    if (sqlite3_exec(userdb,sql,NULL,NULL,&errmsg) != SQLITE_OK) {
                        printf("userdb table insert fail\n"); 
                    }
                }
            }
            else if(usermsg.type == 'L')
            {
                if (sqlite3_exec(userdb,sql_cre_table,NULL,NULL,&errmsg) != SQLITE_OK) {
                    printf("userdb table create fail\n"); 
                }

                memset(sql, 0, sizeof(sql));
                snprintf(sql,sizeof(sql),"SELECT ID FROM userMsg WHERE ID == '%s' ;",usermsg.ID);
                if(sqlite3_exec(userdb,sql,Insert_Check_Callback,&flag_ID,&errmsg) != SQLITE_OK){
                    printf("Insert Check fail\n"); 
                }
                if(flag_ID == 1)
                {
                    flag_ID = 0;
                    snprintf(sql,sizeof(sql),"SELECT ID FROM userMsg WHERE password == '%s' ;" \
                            ,usermsg.password);
                    if (sqlite3_exec(userdb,sql,Insert_Check_Callback,&flag_password,&errmsg) != SQLITE_OK) {
                        printf("userdb table insert fail\n"); 
                    }
                    if (flag_password == 1) //user confirm
                    {
                        memset(sendBuf,0,sizeof(sendBuf)); 
                        sprintf(sendBuf,"Log in success\n");
                        send(aID,sendBuf,strlen(sendBuf)+1,0);    
                        snprintf(sql,sizeof(sql), \
                                "create table if not exists %s (word text, meaning text, time text);",\
                                usermsg.ID);
                        if(sqlite3_exec(hisdb,sql,NULL,NULL,&errmsg) != SQLITE_OK) {
                            printf("hisdb create table fail\n");
                        }  
                        states = 1;
                        strcpy(usermsg_c.ID,usermsg.ID);
                        strcpy(usermsg_c.password, usermsg.password);
                    }
                    else
                    {
                        memset(sendBuf,0,sizeof(sendBuf));
                        sprintf(sendBuf,"Password error\n");
                        send(aID,sendBuf,strlen(sendBuf)+1,0);
                    }
                }
                else
                {
                    memset(sendBuf,0,sizeof(sendBuf)); 
                    sprintf(sendBuf,"Account is not exist\n");
                    send(aID,sendBuf,strlen(sendBuf)+1,0);
                }
            }
        }
        else if(states == 1)
        {
            if(usermsg.type == 'E')
            {
                states = 0;
                memset(sendBuf,0,sizeof(sendBuf));
                sprintf(sendBuf,"Account exit success\n");
                send(aID,sendBuf,strlen(sendBuf)+1,0);
            }else if(usermsg.type == 'S')
            {
                printf("%s\n",usermsg.ID);
                snprintf(sql,sizeof(sql), \
                        "SELECT meaning FROM englishwords WHERE word = '%s';",usermsg.ID);
                if(sqlite3_exec(dictdb,sql,search_Callback,meaning,&errmsg) != SQLITE_OK){
                    printf("word search fail\n");
                }
                if(strcmp(meaning,"NULL!") == 0){
                    memset(sendBuf,0,sizeof(sendBuf));
                    sprintf(sendBuf,"this word is not exist\n");
                    send(aID,sendBuf,strlen(sendBuf)+1,0);
                }else{
                    send(aID,meaning,strlen(meaning)+1,0);
                    time(&current_time);
                    local_time = localtime(&current_time);
                    snprintf(time_str,sizeof(time_str), "%d-%d-%d %d:%d:%d", \
                            local_time->tm_year + 1900,local_time->tm_mon + 1, \
                            local_time->tm_mday, local_time->tm_hour, \
                            local_time->tm_min, local_time->tm_sec);
                    snprintf(sql,sizeof(sql), \
                            "INSERT INTO %s VALUES ('%s', '%s', '%s')", \
                            usermsg_c.ID,usermsg.ID,meaning,time_str);
                    if(sqlite3_exec(hisdb,sql,NULL,NULL,&errmsg) != SQLITE_OK){
                        printf("hist information insert fail\n");
                        printf("%s\n",errmsg);
                    }
                }
            }
        }
    }
    close(aID);
    return 0;
}
