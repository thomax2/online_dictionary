#ifndef __CLIENT_H__
#define __CLIENT_H__

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

int cli_user_register(user_msg* usermsg)
{
    int len;
    printf("Register ID: ");
    fgets(usermsg->ID,sizeof(usermsg->ID),stdin);
    len = strlen(usermsg->ID);
    if(len > 0 && usermsg->ID[len-1] == '\n')
        usermsg->ID[len-1] = '\0';
    printf("Register Password: ");
    fgets(usermsg->password,sizeof(usermsg->password),stdin);
    len = strlen(usermsg->password);
    if(len > 0 && usermsg->password[len-1] == '\n')
        usermsg->password[len-1] = '\0';
}

int cli_user_log(user_msg* usermsg)
{
    int len;
    printf("ID: ");  
    fgets(usermsg->ID,sizeof(usermsg->ID),stdin);
    len = strlen(usermsg->ID);
    if(len > 0 && usermsg->ID[len-1] == '\n')
        usermsg->ID[len-1] = '\0';
    printf("Password: ");
    fgets(usermsg->password,sizeof(usermsg->password),stdin);
    len = strlen(usermsg->password);
    if(len > 0 && usermsg->password[len-1] == '\n')
        usermsg->password[len-1] = '\0';   
}

int cli_user_exit(user_msg* usermsg,int sockcd)
{
    send(sockcd,usermsg,sizeof(*usermsg),0);
    return 0;
}

int cli_search_word(user_msg* usermsg, int sockcd)
{
    int len;
    printf("word: ");  
    fgets(usermsg->ID,sizeof(usermsg->ID),stdin);
    len = strlen(usermsg->ID);
    if(len > 0 && usermsg->ID[len-1] == '\n')
        usermsg->ID[len-1] = '\0';
    send(sockcd,usermsg,sizeof(*usermsg),0);
}

#endif
