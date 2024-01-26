
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

#define INDENT_NEEDED 2
pthread_mutex_t disp_mux;

const int iters = 10;
const int sleep_min = 1;
const int sleep_max = 4;
void displayTab(int n) {
    for(int i = 0; i < n; i++) {
        printf("\t");
    }
}
void* listener(void* arg) {
    static int id = 0;
    int my_id = ++id;

    TQueue* queue = arg;
    pthread_t* me = malloc(sizeof(pthread_t));
    *me = pthread_self();

    sleep(5);
    pthread_mutex_lock(&disp_mux);
        subscribe(queue, me);
        displayTab(my_id * INDENT_NEEDED);
        printf("Sub: %p\n", me);
    pthread_mutex_unlock(&disp_mux);
    while(1) {
        const char* msg = get(queue, me);
        if(strcmp(msg,"abort") == 0)
            break;
        pthread_mutex_lock(&disp_mux);
            displayTab(my_id * INDENT_NEEDED);
            printf("begin(): %s\n", msg);
        pthread_mutex_unlock(&disp_mux);

        sleep(sleep_min + rand() % (sleep_max - sleep_min));

        pthread_mutex_lock(&disp_mux);
            int count = getAvailable(queue, me);
            displayTab(my_id * INDENT_NEEDED);
            printf("end(): %s\n", msg);
        pthread_mutex_unlock(&disp_mux);
    }

    unsubscribe(queue, me);
    return NULL;
}

#define MAX_THREAD_COUNT 128
int main() {
    pthread_mutex_init(&disp_mux, NULL);
    srand(time(NULL));
    TQueue queue;

    const int max_size = 3;
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
        "psa ",
        "tort",
        "rem_rysia",
        "abort"
    };
    sleep(1);
    for(int i = 0; i < messages_count; i++) {
        if(strcmp(messages[i], "add") == 0) {
            pthread_mutex_lock(&disp_mux);
            printf("\t>]Added thread\n");
            pthread_create(&t[thread_idx++], NULL, listener, &queue);
            pthread_mutex_unlock(&disp_mux);
        }else if(strcmp(messages[i], "rem_rysia") == 0) {
            int removed = removeMsg(&queue, "rysia");
            if(removed) {
                pthread_mutex_lock(&disp_mux);
                printf(">!Removed rysia\n");
                pthread_mutex_unlock(&disp_mux);
            }
        }else {
            pthread_mutex_lock(&disp_mux);
            printf("\t>+Putting . . . %s\n",  messages[i]);
            pthread_mutex_unlock(&disp_mux);

            put(&queue, (void*)messages[i]);
            printf(">+Put %s\n",  messages[i]);
        }
    }
    for(int i = 0; i < thread_idx; i++) {
        pthread_join(t[i], NULL);
    }
            
    

    destroyQueue(&queue);
}

