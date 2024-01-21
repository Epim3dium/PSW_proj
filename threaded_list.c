#include "threaded_list.h"
ListNode* createNode(void* msg) {
    ListNode* result = malloc(sizeof(ListNode));
    result->msg = msg;
    result->next = NULL;
    return result;
}
void destroyNode(ListNode* node) {
    free(node);
}
void put(List* list, void* msg) {
    pthread_mutex_lock(&list->mux);
    if(list->size == 0) {
        ListNode* node = createNode(msg);
        list->front = node;
        list->back = node;
        list->size++;
        pthread_mutex_unlock(&list->mux);
        pthread_cond_broadcast(&list->has_elements);
        return;
    }
    //no space
    while(list->size >= list->max_size) {
        pthread_cond_wait(&list->has_free_space, &list->mux);
    }
    list->size++;
    ListNode* node = createNode(msg);
    list->back->next = node;
    list->back = node;
    //space
    pthread_mutex_unlock(&list->mux);
}
//gets from front
void* get(List* list) {
    pthread_mutex_lock(&list->mux);
    while(list->size == 0) {
        pthread_cond_wait(&list->has_elements, &list->mux);
    }
    list->size --;
    void* result = list->front->msg;
    ListNode* temp = list->front;

    list->front = list->front->next;

    destroyNode(temp);
    int hasFreeSpace = 0;
    if(list->size < list->max_size){
        hasFreeSpace = 1;
    }

    pthread_mutex_unlock(&list->mux);
    if(hasFreeSpace)
        pthread_cond_broadcast(&list->has_free_space);
    return result;
}
int removeMsg(List* list, void* msg) {
    pthread_mutex_lock(&list->mux);
    int result = 0;
    if(list->size == 0) {
        pthread_mutex_unlock(&list->mux);
        return result;
    }
    ListNode* ptr = list->front;
    while(ptr->next != NULL) {
        if(ptr->next->msg == msg) {
            result = 1;

            ListNode* temp = ptr->next;
            ptr->next = ptr->next->next;
            destroyNode(temp);

            list->size--;
        }
        ptr = ptr->next;
    }
    pthread_mutex_unlock(&list->mux);
    return result;
}
int getCount(List* list) {
    int result;
    pthread_mutex_lock(&list->mux);
    result = list->size;
    pthread_mutex_unlock(&list->mux);
    return result;
}
void setMaxSize(List* list, const int N) {
    pthread_mutex_lock(&list->mux);
    list->max_size = N;
    pthread_mutex_unlock(&list->mux);
}

int createList(List* list) {
    // if(pthread_mutex_init(&list->back_mux, NULL) != 0) {
    //     printf("list mutex back init has failed\n"); 
    //     return 1;
    // }
    // if(pthread_mutex_init(&list->front_mux, NULL) != 0) {
    //     printf("list mutex front init has failed\n"); 
    //     return 1;
    // }
    if(pthread_mutex_init(&list->mux, NULL) != 0) {
        printf("list mutex front init has failed\n"); 
        return 1;
    }
    if(pthread_cond_init(&list->has_free_space, NULL) != 0) {
        printf("list cond has_free_space init has failed\n"); 
        return 1;
    }
    if(pthread_cond_init(&list->has_elements, NULL) != 0) {
        printf("list cond has_elements init has failed\n"); 
        return 1;
    }
    list->max_size = INT_MAX;
    list->size = 0;
    list->back = NULL;
    return 0;
}

void destroyList(List* list) {
    // pthread_mutex_destroy(&list->front_mux);
    // pthread_mutex_destroy(&list->back_mux);
    pthread_mutex_destroy(&list->mux);
    pthread_cond_destroy(&list->has_elements);
    pthread_cond_destroy(&list->has_free_space);
    ListNode* ptr = list->front;
    while(ptr != NULL) {
        ListNode* temp = ptr;
        ptr = ptr->next;
        destroyNode(temp);
    }
}
