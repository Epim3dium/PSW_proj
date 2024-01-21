#pragma once
#include <limits.h>
#include <pthread.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>


struct TQueueMsgNode {
    //non owning pointer
    void* msg;

    int ref_count;

    //non-owning pointer
    struct TQueueMsgNode* next;
};
typedef struct TQueueMsgNode TQueueMsgNode;

TQueueMsgNode* createMsgNode(void* msg, const int ref_count);
//non recursive
void destroyMsgNode(TQueueMsgNode* node);

struct TQueueThreadNode {
    pthread_t* thread;
    TQueueMsgNode* cur_msg;
    //non-owning pointer
    struct TQueueThreadNode* next;
};
typedef struct TQueueThreadNode TQueueThreadNode;

TQueueThreadNode* createThreadNode(pthread_t* thread, TQueueMsgNode* cur_msg);
//non recursive
void destroyThreadNode(TQueueThreadNode* node);

struct TQueue {
    pthread_mutex_t mux;

    //put()
    pthread_cond_t has_free_space;
    //get()
    pthread_cond_t has_elements;

    int max_size;
    int size;

    int thread_count;

    //non-owning
    TQueueMsgNode* back;
    //owning
    TQueueMsgNode* front;

    //owning
    TQueueThreadNode* thread_head;
};
typedef struct TQueue TQueue;

int createQueue(TQueue* queue, const int size);
void destroyQueue(TQueue* queue);

void subscribe(TQueue* queue, pthread_t* thread);
void unsubscribe(TQueue* queue, pthread_t* thread);
int getAvailable(TQueue *queue, pthread_t *thread);

//at the end, blocking
void put(TQueue* queue, void* msg);
//the oldest(front), removing, blocking
void* get(TQueue* queue, pthread_t* thread);
//searches through whole queue, 0 if didnt find, blocks whole queue(front and back)
int removeMsg(TQueue* queue, void* msg);

int getCount(TQueue* queue);
//non removing
void setSize(TQueue* queue, const int N);
