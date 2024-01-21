
#include <pthread.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>

struct TQueue {
    pthread_mutex_t queue_mux;
    int max_msg_count;
};
typedef struct TQueue TQueue;

int createQueue(TQueue* queue, int msg_count);
void destroyQueue(TQueue* queue);
void subscribe(TQueue* queue, pthread_t* thread);
void unsubscribe(TQueue* queue, pthread_t* thread);

void put(TQueue* queue, void* msg);
void* get(TQueue* queue, pthread_t* thread);
void removeMsg(TQueue* queue, void* msg);

int getAvailable(TQueue* queue, pthread_t* thread);
void setSize(TQueue* queue, int size);


int main() {
    TQueue queue;
}

int createQueue(TQueue* queue, int msg_count) {
    if(pthread_mutex_init(&queue->queue_mux, NULL) != 0) {
        printf("queue mutex init has failed\n"); 
        return 1;
    }
    queue->max_msg_count = msg_count;
    return 0;
}
