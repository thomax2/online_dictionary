#ifndef __LIST_H__
#define __LIST_H__

#include <stdlib.h>

typedef struct LNode *List;
struct LNode{
    char id[20];
    List next;
};

List list_create()
{
    List PtrL;
    PtrL = (List)malloc(sizeof(struct LNode));
    PtrL->next = NULL;
    return PtrL;
}

int Empty(List PtrL)
{
    if(PtrL == NULL)
        return -1;
    return NULL == PtrL->next ? 1:0;
}

List Find(char *newcli_id, List PtrL)
{
    List p = PtrL;
    while (p != NULL && strcmp(newcli_id,p->id) != 0)
        p = p->next;
    return p;
}

int Insert(List PtrL, char* newcli_id)
{
    List p = (List)malloc(sizeof(*PtrL));
    if(p == NULL){
        printf("node gen fail\n");
        return -1; 
    }

    p->next = NULL;
    strcpy(p->id,newcli_id);

    p->next = PtrL->next;
    PtrL->next = p;
    return 0;
}

int Delete(List PtrL, char ecli_id[20])
{
    if(PtrL == NULL || Empty(PtrL))
        return -1;

    List p = PtrL;
    while(strcmp((p->next)->id,ecli_id))
        p = p->next;
    List s = p->next;
    p->next = s->next;
    free(s);
    s=NULL;
    return 0;
}

#endif
