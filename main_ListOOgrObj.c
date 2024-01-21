#include "threaded_list.h"
#include <limits.h>
#include <pthread.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>

const int iters = 10;
const int prod_sleep_min = 1;
const int prod_sleep_max = 4;
void* workerProducer(void* arg) {
    List* list = arg;
    static const char* products[] = {
        "banana",
        "apple",
        "orange",
        "grapefruit",
        "watermelon",
        "lemon"
    };
    static const int product_count = 6;
    for(int i=0; i<iters; i++) {
        sleep(rand() % (prod_sleep_max - prod_sleep_min) + prod_sleep_min);
        const char* msg = products[i % product_count];
        put(list, msg);
        printf("Produced: %s,\tsize: %d\n", msg, list->size);
    }
    return NULL;
}
const int cons_sleep_min = 2;
const int cons_sleep_max = 5;

void* workerConsumer(void* arg) {
    List* list = arg;
    for(int i=0; i<iters; i++) {
        sleep(rand() % (cons_sleep_max - cons_sleep_min) + cons_sleep_min);
        const char* msg = get(list);
        printf("Consumed: %s,\tsize: %d\n", msg, list->size);
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    List list;
    createList(&list);
    setMaxSize(&list, 3);
    pthread_t t1, t2;
    pthread_create(&t1, NULL, workerProducer, &list);
    pthread_create(&t2, NULL, workerConsumer, &list);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    destroyList(&list);
}
