
#include "pub_sub_queue.h"
#include <pthread.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>


// void put(TQueue* queue, void* msg);
// void* get(TQueue* queue, pthread_t* thread);
// void removeMsg(TQueue* queue, void* msg);

// int getAvailable(TQueue* queue, pthread_t* thread);
// void setSize(TQueue* queue, int size);
typedef struct ListenerInfo ListenerInfo;

const int iters = 10;
const int sleep_min = 1;
const int sleep_max = 4;
void* listener(void* arg) {
    TQueue* queue = arg;
    pthread_t* me = malloc(sizeof(pthread_t));
    *me = pthread_self();

    subscribe(queue, me);
    printf("Subscribed\tby: %p\n", me);
    while(1) {
        sleep(sleep_min + rand() % (sleep_max - sleep_min));
        const char* msg = get(queue, me);
        int count = getAvailable(queue, me);
        printf("Processed: %s,\tby: %p,\tmsg available:%d\n", msg, (void*)me, count);
        if(strcmp(msg,"abort") == 0)
            break;
    }

    unsubscribe(queue, me);
    return NULL;
}

#define MAX_THREAD_COUNT 128
int main() {
    srand(time(NULL));
    TQueue queue;

    const int max_size = 5;
    createQueue(&queue, max_size);

    int thread_idx = 0;
    pthread_t t[MAX_THREAD_COUNT];

    pthread_create(&t[thread_idx++], NULL, listener, &queue);
    pthread_create(&t[thread_idx++], NULL, listener, &queue);
#define messages_count 11
    //abort - koniec watku
    //add - dodaj watek
    //rem_rysia - usuwa rysia
    const char* messages[messages_count] = {
        "ala ",
        "ma  ",
        "kota",
        "sierotka",
        "posiada",
        "add",
        "rysia",
        "psa",
        "tort",
        "rem_rysia",
        "abort"
    };
    for(int i = 0; i < messages_count; i++) {
        if(strcmp(messages[i], "add") == 0) {
            pthread_create(&t[thread_idx++], NULL, listener, &queue);
            printf("\t\t>]Added thread\n");
        }else if(strcmp(messages[i], "rem_rysia") == 0) {
            int removed = removeMsg(&queue, "rysia");
            if(removed) {
                printf("\t\t>!Removed rysia\n");
            }
        }else {
            put(&queue, messages[i]);
            printf("\t\t>+Put %s\n",  messages[i]);
        }
    }
    for(int i = 0; i < thread_idx; i++) {
        pthread_join(t[i], NULL);
    }
            
    

    destroyQueue(&queue);
}

