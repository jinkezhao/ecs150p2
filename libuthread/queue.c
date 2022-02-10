#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"


/**
 * node struct for the queue
 * Note that it is a singly linked list implementation
 */
typedef struct element {
    void *data;
    struct element *next;
}* element_t;


/**
 * queue with a head, tail pointer and a size property
 */
struct queue {
	/* TODO */
    element_t head;
    element_t tail;
    int size;
};

/**
 * allocate memory for a new node and initialize it
 * @param data
 * @return
 */
element_t make_new_element(void* data){
    element_t new_element = (element_t)malloc(sizeof(struct element));
    if(new_element != NULL){
        new_element->data = data;
        new_element->next = NULL;
        return new_element;
    }
    else{
        return NULL;
    }
}

queue_t queue_create(void)
{
	/* TODO */
    queue_t new_queue = (queue_t)malloc(sizeof(struct queue));
    if(new_queue == NULL){
        return NULL;
    }
    new_queue->size = 0;
    new_queue->head = NULL;
    new_queue->tail = NULL;
	return new_queue;
}

int queue_destroy(queue_t queue)
{
	/* TODO */
    if(queue == NULL || queue->size > 0) {
        return -1;
    }
    else{
        free(queue);
        return 0;
    }
}

int queue_enqueue(queue_t queue, void *data)
{
	/* TODO */
    if(queue == NULL || data == NULL) {
        return -1;
    }
    else{
        element_t  new_element = make_new_element(data);
        if(new_element == NULL){
            return -1;
        }
        else{
            if(queue->size == 0){
                queue->head = new_element;
                queue->tail = new_element;

            }
            else{
                queue->tail->next = new_element;
                queue->tail = new_element;
            }
            ++queue->size;
            return 0;
        }
    }
}

int queue_dequeue(queue_t queue, void **data)
{
	/* TODO */
    if(queue == NULL || data == NULL || queue->size == 0) {
        return -1;
    }
    else{
        element_t  cur = queue->head;
        *data = queue->head->data;
        queue->head = queue->head->next;
        if(queue->size == 1){
            queue->tail = NULL;
        }
        free(cur);
        --queue->size;
        return 0;
    }
}

int queue_delete(queue_t queue, void *data)
{
	/* TODO */
    if(queue == NULL || data == NULL || queue->size == 0) {
        return -1;
    }
    else{
        element_t prev = queue->head;
        element_t cur = queue->head;
        int found = 0;
        while(cur != NULL){
            if(cur->data == data){
                found = 1;
                break;
            }
            else {
                prev = cur;
                cur = cur->next;
            }
        }
        if(found){
            /**
             * if data is equal to the date in the first element of the queue
             */
            if(prev == cur){
                if(queue->size == 1){
                    queue->tail = NULL;
                }
                queue->head = queue->head->next;
                free(cur);
            }
            /**
             * cur element holds the data
             */
            else{
                prev->next = cur->next;
                /**
                 * if the data is equal to the data in queue->tail
                 */
                if(cur == queue->tail){
                    queue->tail = prev;
                }
                cur->next = NULL;
                free(cur);
            }
            --queue->size;
            return 0;
        }
        else{
            return -1;
        }
    }
}

int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data)
{
	/* TODO */
    if(queue == NULL || func == NULL) {
        return -1;
    }
    else{
        int status = 0;
        element_t cur = queue->head;
        element_t next;
        if(queue->head != NULL) {
            next = queue->head->next;
        }
        else{
            next = NULL;
        }
        while(cur!= NULL  &&  status != 1){
            status = func(queue, cur->data, arg);
            if(status == 1 && data != NULL ){
                *data = cur->data;
                break;
            }
            cur = next;
            if(cur != NULL){
                next = cur->next;
            }
        }
        return 0;
    }
}

int queue_length(queue_t queue)
{
	/* TODO */
    if(queue == NULL) {
        return -1;
    }
    else{
        return queue->size;
    }
}

