#include <limits.h>
#include <pthread.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>

struct Node {
    //non owning pointer
    void* msg;

    //owning pointer
    struct Node* next;
};
typedef struct Node Node;

Node* createNode(void* msg) {
    Node* result = malloc(sizeof(Node));
    result->msg = msg;
    result->next = NULL;
    return result;
}
//non recursive
void destroyNode(Node* node) {
    free(node);
}

struct Queue {
    pthread_mutex_t mux;

    //put()
    pthread_cond_t has_free_space;
    //get()
    pthread_cond_t has_elements;

    int max_size;
    int size;

    //non-owning
    Node* back;
    //owning
    Node* front;
};
typedef struct Queue Queue;

int createQueue(Queue* queue);
void destroyQueue(Queue* queue);

//at the end, blocking
void put(Queue* queue, void* msg);
//the oldest(front), removing, blocking
void* get(Queue* queue);
//searches through whole list, 0 if didnt find, blocks whole queue(front and back)
int removeMsg(Queue* queue, void* msg);

//goes through whole/counts 1 by 1
int getCount(Queue* queue);
//non removing
void setMaxSize(Queue* queue, const int N);

const int iters = 10;

const int prod_sleep_min = 1;
const int prod_sleep_max = 5;

void* workerProducer(void* arg) {
    Queue* queue = arg;
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
        put(queue, msg);
        printf("Produced: %s,\tsize: %d\n", msg, queue->size);
    }
    return NULL;
}
const int cons_sleep_min = 2;
const int cons_sleep_max = 7;

void* workerConsumer(void* arg) {
    Queue* queue = arg;
    for(int i=0; i<iters; i++) {
        sleep(rand() % (cons_sleep_max - cons_sleep_min) + cons_sleep_min);
        const char* msg = get(queue);
        printf("Consumed: %s,\tsize: %d\n", msg, queue->size);
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    Queue queue;
    createQueue(&queue);
    pthread_t t1, t2;
    pthread_create(&t1, NULL, workerProducer, &queue);
    pthread_create(&t2, NULL, workerConsumer, &queue);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    destroyQueue(&queue);
}
void put(Queue* queue, void* msg) {
    pthread_mutex_lock(&queue->mux);
    if(queue->size == 0) {
        Node* node = createNode(msg);
        queue->front = node;
        queue->back = node;
        queue->size++;
        pthread_mutex_unlock(&queue->mux);
        pthread_cond_broadcast(&queue->has_elements);
        return;
    }
    //no space
    while(queue->size >= queue->max_size) {
        pthread_cond_wait(&queue->has_free_space, &queue->mux);
    }
    queue->size++;
    Node* node = createNode(msg);
    queue->back->next = node;
    queue->back = node;
    //space
    pthread_mutex_unlock(&queue->mux);
}
//gets from front
void* get(Queue* queue) {
    pthread_mutex_lock(&queue->mux);
    while(queue->size == 0) {
        pthread_cond_wait(&queue->has_elements, &queue->mux);
    }
    queue->size --;
    void* result = queue->front->msg;
    Node* temp = queue->front;

    queue->front = queue->front->next;

    destroyNode(temp);
    int hasFreeSpace = 0;
    if(queue->size < queue->max_size){
        hasFreeSpace = 1;
    }

    pthread_mutex_unlock(&queue->mux);
    if(hasFreeSpace)
        pthread_cond_broadcast(&queue->has_free_space);
    return result;
}
int removeMsg(Queue* queue, void* msg) {
    pthread_mutex_lock(&queue->mux);
    int result = 0;
    if(queue->size == 0) {
        pthread_mutex_unlock(&queue->mux);
        return result;
    }
    Node* ptr = queue->front;
    while(ptr->next != NULL) {
        if(ptr->next->msg == msg) {
            result = 1;

            Node* temp = ptr->next;
            ptr->next = ptr->next->next;
            destroyNode(temp);

            queue->size--;
        }
        ptr = ptr->next;
    }
    pthread_mutex_unlock(&queue->mux);
    return result;
}
int getCount(Queue* queue) {
    int result;
    pthread_mutex_lock(&queue->mux);
    result = queue->size;
    pthread_mutex_unlock(&queue->mux);
    return result;
}
void setMaxSize(Queue* queue, const int N) {
    pthread_mutex_lock(&queue->mux);
    queue->max_size = N;
    pthread_mutex_unlock(&queue->mux);
}

int createQueue(Queue* queue) {
    // if(pthread_mutex_init(&queue->back_mux, NULL) != 0) {
    //     printf("queue mutex back init has failed\n"); 
    //     return 1;
    // }
    // if(pthread_mutex_init(&queue->front_mux, NULL) != 0) {
    //     printf("queue mutex front init has failed\n"); 
    //     return 1;
    // }
    if(pthread_mutex_init(&queue->mux, NULL) != 0) {
        printf("queue mutex front init has failed\n"); 
        return 1;
    }
    if(pthread_cond_init(&queue->has_free_space, NULL) != 0) {
        printf("queue cond has_free_space init has failed\n"); 
        return 1;
    }
    if(pthread_cond_init(&queue->has_elements, NULL) != 0) {
        printf("queue cond has_elements init has failed\n"); 
        return 1;
    }
    queue->max_size = INT_MAX;
    queue->size = 0;
    queue->back = NULL;
    return 0;
}

void destroyQueue(Queue* queue) {
    // pthread_mutex_destroy(&queue->front_mux);
    // pthread_mutex_destroy(&queue->back_mux);
    pthread_mutex_destroy(&queue->mux);
    pthread_cond_destroy(&queue->has_elements);
    pthread_cond_destroy(&queue->has_free_space);
    Node* ptr = queue->front;
    while(ptr != NULL) {
        Node* temp = ptr;
        ptr = ptr->next;
        destroyNode(temp);
    }
}
