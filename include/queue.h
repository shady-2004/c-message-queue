#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>
#include <stdbool.h>

//Return & Error codes
typedef enum {
    QUEUE_SUCCESS       =  0,
    QUEUE_ERR_INVALID   = -1,
    QUEUE_ERR_NOMEM     = -2,
    QUEUE_ERR_SHUTDOWN  = -3
} queue_result_t;


typedef struct queue queue_t;

//Allocate a bounded queue with fixed size , return null on failure 
queue_t *queue_create(size_t capacity);

//Pushes an item into the queue , Blocks if the queue is full
// Returns QUEUE_SUCCESS or QUEUE_ERR_SHUTDOWN if stopped while waiting
queue_result_t queue_push(queue_t *q , void*item) ;

//Pops an item from the queue into *item_out. Blocks if empty.
// Returns QUEUE_SUCCESS or QUEUE_ERR_SHUTDOWN if stopped while waiting
queue_result_t queue_pop(queue_t *q, void **item_out);

//Wakes all sleeping producer and consumer threads to exit cleanly
void queue_shutdown(queue_t *q);

//Frees memory , destroy locks and free queue
void queue_destroy(queue_t *q);


#endif