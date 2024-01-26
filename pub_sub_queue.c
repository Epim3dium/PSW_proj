
#include "pub_sub_queue.h"
#include <assert.h>
TQueueMsgNode* createMsgNode(void* msg, const int ref_count) {
    TQueueMsgNode* result = malloc(sizeof(TQueueMsgNode));
    result->msg = msg;
    result->ref_count = ref_count;
    result->next = NULL;
    return result;
}
void destroyMsgNode(TQueueMsgNode* node) {
    free(node);
}
TQueueThreadNode* createThreadNode(pthread_t* thread, TQueueMsgNode* cur_msg) {
    TQueueThreadNode* result = malloc(sizeof(TQueueThreadNode));
    result->next = NULL;
    result->thread = thread;
    result->cur_msg = cur_msg;
    return result;
}
//non recursive
void destroyThreadNode(TQueueThreadNode* node) {
    TQueueMsgNode* mnode = node->cur_msg;
    while(mnode != NULL) {
        mnode->ref_count --;
        mnode = mnode->next;
    }
    free(node);
}
void subscribe(TQueue* queue, pthread_t* thread) {
    pthread_mutex_lock(&queue->mux);
    queue->thread_count++;
    TQueueThreadNode* node = createThreadNode(thread, NULL);
    node->next = queue->thread_head;
    queue->thread_head = node;
    pthread_mutex_unlock(&queue->mux);
}
void unsubscribe(TQueue* queue, pthread_t* thread) {
    pthread_mutex_lock(&queue->mux);
    TQueueThreadNode* tnode = queue->thread_head;
    if(tnode == NULL) {
        pthread_mutex_unlock(&queue->mux);
        return;
    }

    if(tnode->thread == thread) {
        queue->thread_head = tnode->next;
        destroyThreadNode(tnode);
        queue->thread_count--;
    }
    while(tnode->next != NULL) {
        if(tnode->next->thread == thread) {
            TQueueThreadNode* temp = tnode->next;
            tnode->next = tnode->next->next;
            destroyThreadNode(temp);
            queue->thread_count--;
            break;
        }
        tnode = tnode->next;
    }
    //check if unsubscriber was the last listener
    while(queue->front != NULL && queue->front->ref_count == 0) {
        TQueueMsgNode* temp = queue->front;
        queue->front = queue->front->next;
        destroyMsgNode(temp);
        queue->size--;
    }
    if(queue->size < queue->max_size){
        pthread_cond_broadcast(&queue->has_free_space);
    }
    pthread_mutex_unlock(&queue->mux);
}
int getAvailable(TQueue *queue, pthread_t *thread) {
    pthread_mutex_lock(&queue->mux);
    TQueueThreadNode* node = queue->thread_head;
    TQueueMsgNode* msg_node = NULL;
    //search for correspoing node
    while(node != NULL) {
        if(node->thread == thread) {
            msg_node = node->cur_msg;
            break;
        }
        node = node->next;
    }
    //count messages
    int result = 0;
    while(msg_node != NULL) {
        result++;
        msg_node = msg_node->next;
    }
    pthread_mutex_unlock(&queue->mux);
    return result;
}

//returns 1 if had to update
static int update_missing_msgs(TQueueThreadNode* head, TQueueMsgNode* new_msg){
    TQueueThreadNode* thread_node = head;
    int had_to_update = 0;
    while(thread_node != NULL) {
        if(thread_node->cur_msg == NULL) {
            had_to_update = 1;
            thread_node->cur_msg = new_msg;
        }
        thread_node = thread_node->next;
    }
    return had_to_update;
}

void put(TQueue* queue, void* msg) {
    pthread_mutex_lock(&queue->mux);
    if(queue->thread_count == 0) {
        pthread_mutex_unlock(&queue->mux);
        return;
    }
    if(queue->size == 0) {
        TQueueMsgNode* new_msg_node = createMsgNode(msg, queue->thread_count);
        queue->front = new_msg_node;
        queue->back = new_msg_node;
        queue->size++;
        update_missing_msgs(queue->thread_head, new_msg_node);
        pthread_cond_broadcast(&queue->has_elements);
        pthread_mutex_unlock(&queue->mux);
        return;
    }
    //no space
    while(queue->size >= queue->max_size) {
        pthread_cond_wait(&queue->has_free_space, &queue->mux);
    }
    TQueueMsgNode* new_msg_node = createMsgNode(msg, queue->thread_count);
    queue->back->next = new_msg_node;
    queue->back = new_msg_node;
    queue->size++;

    //adding msg to these threads that didnt get it
    if(update_missing_msgs(queue->thread_head, new_msg_node)) {
        pthread_cond_broadcast(&queue->has_elements);
    }
    
    pthread_mutex_unlock(&queue->mux);
}
//gets from front
void* get(TQueue* queue, pthread_t* thread) {
    pthread_mutex_lock(&queue->mux);
    TQueueThreadNode* node = queue->thread_head;
    //getting node with thread* arg
    while(node != NULL && node->thread != thread) {
        node = node->next;
    }
    assert(node != NULL);

    //waiting for message
    while(node->cur_msg == NULL) {
        pthread_cond_wait(&queue->has_elements, &queue->mux);
    }
    TQueueMsgNode* mnode = node->cur_msg;
    node->cur_msg = node->cur_msg->next;

    void* result = mnode->msg;
    mnode->ref_count--;
    //remove message if you were last subscriber
    if(mnode->ref_count == 0) {
        TQueueMsgNode* temp = queue->front;
        queue->front = queue->front->next;
        destroyMsgNode(temp);
        queue->size--;
        if(queue->size < queue->max_size){
            pthread_cond_signal(&queue->has_free_space);
        }
    }


    pthread_mutex_unlock(&queue->mux);
    return result;
}
int removeMsg(TQueue* queue, void* msg) {
    pthread_mutex_lock(&queue->mux);
    if(queue->size == 0) {
        pthread_mutex_unlock(&queue->mux);
        return 0;
    }
    TQueueMsgNode* ptr = queue->front;
    TQueueMsgNode* to_destroy = NULL;
    if(ptr->msg == msg) {
        to_destroy = ptr;
        queue->front = queue->front->next;
        queue->size--;
    }else {
        while(ptr->next != NULL) {
            if(ptr->next->msg != msg) {
                ptr = ptr->next;
                continue;
            }

            to_destroy = ptr->next;
            ptr->next = ptr->next->next;

            queue->size--;
            break;
        }
    }
    if(queue->size < queue->max_size){
        pthread_cond_broadcast(&queue->has_free_space);
    }
    int result = 0;
    if(to_destroy != NULL) {
        result = 1;
        TQueueThreadNode* tnode = queue->thread_head;
        while(tnode != NULL) {
            if(tnode->cur_msg == to_destroy) {
                tnode->cur_msg = tnode->cur_msg->next;
            }
            tnode = tnode->next;
        }
        destroyMsgNode(to_destroy);
    }
    pthread_mutex_unlock(&queue->mux);
    return result;
}
int getCount(TQueue* queue) {
    int result;
    pthread_mutex_lock(&queue->mux);
    result = queue->size;
    pthread_mutex_unlock(&queue->mux);
    return result;
}
void setSize(TQueue* queue, const int N) {
    pthread_mutex_lock(&queue->mux);
    queue->max_size = N;
    if(queue->size < queue->max_size){
        pthread_cond_broadcast(&queue->has_free_space);
    }
    pthread_mutex_unlock(&queue->mux);
}

int createQueue(TQueue* queue, const int size) {
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
    queue->thread_head = NULL;
    queue->thread_count = 0;
    setSize(queue, size);
    return 0;
}

void destroyQueue(TQueue* queue) {
    // pthread_mutex_destroy(&queue->front_mux);
    // pthread_mutex_destroy(&queue->back_mux);
    pthread_mutex_destroy(&queue->mux);
    pthread_cond_destroy(&queue->has_elements);
    pthread_cond_destroy(&queue->has_free_space);

    TQueueThreadNode* tptr = queue->thread_head;
    while(tptr != NULL) {
        TQueueThreadNode* temp = tptr;
        tptr = tptr->next;
        destroyThreadNode(temp);
    }
    TQueueMsgNode* mptr = queue->front;
    while(mptr != NULL) {
        TQueueMsgNode* temp = mptr;
        mptr = mptr->next;
        destroyMsgNode(temp);
    }
}
