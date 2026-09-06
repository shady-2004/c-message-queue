#include "queue.h"

#include <stdlib.h>
#include <pthread.h>

struct queue{
    void **buffer;      //Dynamic array 
    size_t capacity;    //Maximum capacity
    size_t head;        //Read index
    size_t tail;        //Write index
    size_t count;       //Number of items in buffer

    pthread_mutex_t lock; //Mutex for shared queue state
    pthread_cond_t not_full; //Signaled when item popped
    pthread_cond_t not_empty; //Signaled when item pushed
    
    bool is_shutdown; //Flag to releas blocked threads
};

queue_t *queue_create(size_t capacity) {
    if (capacity == 0) return NULL;

    queue_t *q = malloc(sizeof(queue_t)); 

    q->buffer = malloc(sizeof(void *) * capacity);
    
    if (!q->buffer) {
        free(q);
        return NULL;
    }

    q->capacity = capacity;
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    q->is_shutdown = false;

    if (pthread_mutex_init(&q->lock, NULL) != 0){
        goto err_free_buffer;
    }

    if (pthread_cond_init(&q->not_full,NULL) != 0 ){
        goto err_destroy_mutex;
    }

    if (pthread_cond_init(&q->not_empty,NULL) != 0 ){
        goto err_destroy_not_full;
    }

    return q;


//Handle initialization errors
err_destroy_not_full:
    pthread_cond_destroy(&q->not_full);

err_destroy_mutex:
    pthread_mutex_destroy(&q->lock);

err_free_buffer:
    free(q->buffer);
    free(q);
    return NULL;

}

queue_result_t queue_push(queue_t *q , void*item){
    if (!q) return QUEUE_ERR_INVALID;

    pthread_mutex_lock(&q->lock);

    // Wail while buffer is full and queue is operating
    while (q->count == q->capacity && !q->is_shutdown){
        pthread_cond_wait(&q->not_full, &q->lock);
    }

    if (q->is_shutdown) { 
        pthread_mutex_unlock(&q->lock);
        return QUEUE_ERR_SHUTDOWN;
    }

    //Place item and the end and advance the tail
    q->buffer[q->tail] = item;
    q->tail = ( q->tail + 1 ) %  q->capacity;
    q->count++;

    //Signal for waiting consumers 
    pthread_cond_signal(&q->not_empty);

    pthread_mutex_unlock(&q->lock);
    return QUEUE_SUCCESS;

}

queue_result_t queue_pop(queue_t *q , void**item_out){
    if (!q || !item_out) return QUEUE_ERR_INVALID;

    pthread_mutex_lock(&q->lock);

    // Wail while buffer is empty and queue is operating
    while (q->count == 0 && !q->is_shutdown){
        pthread_cond_wait(&q->not_empty, &q->lock);
    }

    if (q->is_shutdown) { 
        pthread_mutex_unlock(&q->lock);
        return QUEUE_ERR_SHUTDOWN;
    }

    //Extract item and advance head
    *item_out = q->buffer[q->head];
    q->head = ( q->head + 1 ) %  q->capacity;
    q->count--;

    //Signal for waiting producer
    pthread_cond_signal(&q->not_full);

    pthread_mutex_unlock(&q->lock);
    return QUEUE_SUCCESS;

}