#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "sqlite3.h"
#include <time.h>
#include <pthread.h>
#include "list.h"

typedef struct sockaddr sockaddr;
typedef struct user_msg{
    char type;
    char ID[20];
    char password[20];
}user_msg;

typedef struct msg{
    List L;
    int newfd;
}msg;

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


int user_register(sqlite3* userdb, user_msg usermsg, int aID)
{
    char* sql_cre_table = "create table if not exists userMsg (id text primary key, password text);"; 
    char sql[200];
    char sendBuf[200];
    char* errmsg = NULL;
    int flag_ID = 0;
    
    if (sqlite3_exec(userdb,sql_cre_table,NULL,NULL,&errmsg) != SQLITE_OK) {
         printf("userdb table create fail\n"); 
    }

    snprintf(sql,sizeof(sql),"SELECT ID FROM userMsg WHERE ID == '%s' ;",usermsg.ID);
    if(sqlite3_exec(userdb,sql,Insert_Check_Callback,&flag_ID,&errmsg) != SQLITE_OK){
        printf("Insert Check fail\n"); 
    }
    
    if(flag_ID == 1)
    {
        flag_ID = 0;
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
    return 0;
}


int user_log(sqlite3 *userdb, sqlite3 *hisdb, user_msg usermsg, int aID, user_msg* usermsg_c, List PtrL)
{
    char* errmsg = NULL;
    char* sql_cre_table = "create table if not exists userMsg (id text primary key, password text);";
    char sendBuf[200];
    char sql[200];
    int flag_ID = 0;
    int flag_password = 0;
   
    if(Find(usermsg.ID,PtrL) != NULL){
        sprintf(sendBuf,"Account is already Log\n");
        send(aID,sendBuf,strlen(sendBuf)+1,0);
        return 0;
    }

    if (sqlite3_exec(userdb,sql_cre_table,NULL,NULL,&errmsg) != SQLITE_OK) {
        printf("userdb table create fail\n");
    }
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
            Insert(PtrL,usermsg.ID);
            sprintf(sendBuf,"Log in success\n");
            send(aID,sendBuf,strlen(sendBuf)+1,0);    
            snprintf(sql,sizeof(sql), \
                    "create table if not exists %s (word text, meaning text, time text);",\
                    usermsg.ID);
            if(sqlite3_exec(hisdb,sql,NULL,NULL,&errmsg) != SQLITE_OK)
                printf("hisdb create table fail\n");
            strcpy(usermsg_c->ID,usermsg.ID);
            strcpy(usermsg_c->password, usermsg.password);
            return 1;
        }
        else
        {
            sprintf(sendBuf,"Password error\n");
            send(aID,sendBuf,strlen(sendBuf)+1,0);
        }
    }
    else
    {
        sprintf(sendBuf,"Account is not exist\n");
        send(aID,sendBuf,strlen(sendBuf)+1,0);
    }
    return 0;
}

int user_exit(List PtrL, int aID, char *ecli_id)
{
    char sendBuf[200];
    Delete(PtrL,ecli_id);
    sprintf(sendBuf,"Account exit success\n");
    send(aID,sendBuf,strlen(sendBuf)+1,0);
    return 0;
}

int word_search(sqlite3 *dictdb, sqlite3 *hisdb, user_msg usermsg, user_msg usermsg_c,int aID)
{
    char sql[200];
    char meaning[80];
    char* errmsg = NULL;
    char time_str[50];
    char sendBuf[200];
    time_t current_time;
    struct tm *local_time;
    printf("%s\n",usermsg.ID);
    snprintf(sql,sizeof(sql), \
            "SELECT meaning FROM englishwords WHERE word = '%s';",usermsg.ID);
    if(sqlite3_exec(dictdb,sql,search_Callback,meaning,&errmsg) != SQLITE_OK){
        printf("word search fail\n");
    }
    if(strcmp(meaning,"NULL!") == 0){
        sprintf(sendBuf,"this word is not exist\n");
        send(aID,sendBuf,strlen(sendBuf)+1,0);
    }
    else{
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
    return 1;
}

int user_inf_fetch(user_msg* usermsg, int aID)
{
    int Recv_len;
    Recv_len = recv(aID,usermsg,sizeof(*usermsg),0); //receive data from client
    printf("%d\n",Recv_len);
    if(Recv_len > 0){
        //printf("%s\t%s\n",usermsg->ID,usermsg->password);
        return 1;
    }
    else
        return 0; 
}

void* rcv_cli_proc(void* arg)
{
    pthread_detach(pthread_self());
    int states =0; //0 user uncertain; 1 user certain;
    user_msg usermsg;
    user_msg usermsg_c;
    int Recv_len;
    char recBuf[200]; // receive data from client
   
    int aID = ((msg *)arg)->newfd;
    List PtrL = ((msg *)arg)->L;

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
   
    while(1)
    {
        if(!user_inf_fetch(&usermsg,aID))
            break;
        if(states == 0){
            if(usermsg.type == 'R'){
                states = user_register(userdb, usermsg, aID);
            }
            else if(usermsg.type == 'L'){
                states = user_log(userdb,hisdb,usermsg,aID,&usermsg_c,PtrL);
            }
        }
        else if(states == 1){
            if(usermsg.type == 'E'){
                states = user_exit(PtrL,aID,usermsg_c.ID);
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
    pthread_exit(NULL);
}

#endif
