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