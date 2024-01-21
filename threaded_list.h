#pragma once
#include <limits.h>
#include <pthread.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>


struct ListNode {
    //non owning pointer
    void* msg;

    //owning pointer
    struct ListNode* next;
};
typedef struct ListNode ListNode;

ListNode* createNode(void* msg);
//non recursive
void destroyNode(ListNode* node);

struct List {
    pthread_mutex_t mux;

    //put()
    pthread_cond_t has_free_space;
    //get()
    pthread_cond_t has_elements;

    int max_size;
    int size;

    //non-owning
    ListNode* back;
    //owning
    ListNode* front;
};
typedef struct List List;

int createList(List* list);
void destroyList(List* list);

//at the end, blocking
void put(List* list, void* msg);
//the oldest(front), removing, blocking
void* get(List* list);
//searches through whole list, 0 if didnt find, blocks whole list(front and back)
int removeMsg(List* list, void* msg);

int getCount(List* list);
//non removing
void setMaxSize(List* list, const int N);
