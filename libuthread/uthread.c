#include <assert.h>
#include <limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"
#define FAIL -1
#define SUCCEED 0
/* TODO */


/**
 * holding the next available global uthread_t
 */
uthread_t current_max_tid;

/**
 * state of the thread
 */
typedef enum State{
    UNINITIALIZED, READY, RUNNING, BLOCKED, ZOMBIE
}State;

/**
 * pointer to TCB
 */
typedef struct TCB* TCB_t;


/**
 * Thread Control Block
 */
struct TCB{
    uthread_t tid;
    State state; // state of the thread
    uthread_ctx_t* context; // context of the thread
    void* sp; // top of the stack
    int retval; //exit status of the thread
    TCB_t joined_by; // thread to join this thread
    TCB_t join; // thread that this thread joins
};

//queue to hold ready threads
queue_t ready_queue;
//queue to hold blocked threads
queue_t blocked_queue;
//queue to hold zombie thrreads
queue_t zombie_queue;

//the current active thrad
TCB_t  current_thread;
//allocate memory for TCB
TCB_t make_new_TCB();
//check of uthread_t overflow
int tid_overflow();
//initialize the context of the thread with the func
int init_context(TCB_t tcb_t, uthread_func_t func);
//free the memory of the TCB_t
void free_TCB_t(TCB_t tcb_t);
//find the TCB_t by its tid in ready_queue, blocked_queue and zombie queue
int find_TCB_by_tid(uthread_t tid, TCB_t* tcb_t_t);
int find_TCB_t(queue_t q, void *data, void *arg);


int uthread_start(int preempt)
{
    /* TODO */
    current_max_tid = 0;
    ready_queue = queue_create();
    if(ready_queue == NULL){
        return FAIL;
    }

    blocked_queue = queue_create();
    if(blocked_queue == NULL){
        return FAIL;
    }

    zombie_queue = queue_create();
    if(zombie_queue == NULL){
        return FAIL;
    }
    current_thread = make_new_TCB();
    if(current_thread == NULL){
        return FAIL;
    }
    current_thread->state = RUNNING;
    if(init_context(current_thread, NULL) == FAIL){
        free_TCB_t(current_thread);
        current_thread = NULL;
        return FAIL;
    }
    if(preempt){
        preempt_start();
    }
    return SUCCEED;
}

int uthread_stop(void)
{
    /* TODO */
    //should be called by the main thread only
    if(current_thread->tid != 0){
        fprintf(stderr, "Warning: uthread_stop is not called by the main thread but by tid: %u\n", current_thread->tid);
        return FAIL;
    }
    //should stop if there are no more user threads
    if(queue_length(ready_queue) == 0 && queue_length(blocked_queue) == 0){
        free(ready_queue);
        free(blocked_queue);
        if(queue_length(zombie_queue) > 0){
            TCB_t tcb_t;
            TCB_t* tcb_t_t = &tcb_t;
            while(queue_length(zombie_queue) > 0){
                if(queue_dequeue(zombie_queue, (void**)tcb_t_t) == SUCCEED){
                    free(*tcb_t_t);
                }
            }
        }
        free(zombie_queue);
        free(current_thread);
        ready_queue = NULL;
        blocked_queue = NULL;
        zombie_queue = NULL;
        current_thread = NULL;
        return SUCCEED;
    }
    else{
        fprintf(stderr, "Warning: there are still user threads in ready queue or blocked queue\n");
        return FAIL;
    }
}

int uthread_create(uthread_func_t func)
{
    /* TODO */
    if(tid_overflow()){
        return FAIL;
    }
    preempt_disable();
    TCB_t  new_tcb_t = make_new_TCB();
    preempt_enable();
    if(new_tcb_t == NULL){
        return FAIL;
    }
   if(init_context(new_tcb_t, func) == FAIL){
       return FAIL;
   }
   new_tcb_t->state = READY;
   preempt_disable();
    if(queue_enqueue(ready_queue, new_tcb_t) == SUCCEED){
        preempt_enable();
        return new_tcb_t->tid;
    }
    else{
        preempt_enable();
        free_TCB_t(new_tcb_t);
        return FAIL;
    }
}

void uthread_yield(void)
{
    /* TODO */
    TCB_t next_thread, prev_thread;
    preempt_disable();
    //if a ready thread is found in the ready queue
    if(queue_dequeue(ready_queue, (void**)&next_thread) == SUCCEED){
        if(current_thread->state == RUNNING){
            current_thread->state = READY;
            queue_enqueue(ready_queue, current_thread);
        }
        prev_thread = current_thread;
        current_thread = next_thread;
        current_thread->state = RUNNING;
        uthread_ctx_switch(prev_thread->context, next_thread->context);
        preempt_enable();
    }
    //otherwise, do nothing, so the current_thread is not changed
    preempt_enable();
}

uthread_t uthread_self(void)
{
    /* TODO */
    uthread_t cur_tid;
    preempt_disable();
    cur_tid = current_thread->tid;
    preempt_enable();
    return cur_tid;
}

void uthread_exit(int retval)
{
    /* TODO */
    //save the return value so that can be collected later
    preempt_disable();
    current_thread->retval = retval;
    current_thread->state = ZOMBIE;
    queue_enqueue(zombie_queue, current_thread);
    if(current_thread->joined_by != NULL){
        if(current_thread->joined_by->state == BLOCKED) {
            //remove the joining thread from the blocked queue
            queue_delete(blocked_queue, current_thread->joined_by);
        }
        current_thread->joined_by->state = READY;
        //put the thread that joins current_thread into ready_queue
        queue_enqueue(ready_queue,  current_thread->joined_by);
    }
    preempt_enable();
    uthread_yield();
}

int uthread_join(uthread_t tid, int *retval)
{
    /* TODO */
    // main thread cannot be joined and a thread cannot join itself
    preempt_disable();
    if(tid == 0 || tid == current_thread->tid){
        return FAIL;
    }
    TCB_t target;
    TCB_t * tcb_t_t = &target;
    *tcb_t_t = NULL;
    int found = find_TCB_by_tid(tid, tcb_t_t);
    // if the thread cannot be found
    if(found == FAIL){
        return FAIL;
    }
    //if the thread is already joined
    if(target->joined_by != NULL){
        return FAIL;
    }
    //if the thread is already dead
    if(target->state == ZOMBIE){
        if(retval != NULL) {
            *retval = target->retval;
        }
        queue_delete(zombie_queue, target);
        free_TCB_t(target);
        target = NULL;
    }
    else {
        target->joined_by = current_thread;
        current_thread->join = target;
        while(1) {
            preempt_disable();
            if(target->state != ZOMBIE) {
                current_thread->state = BLOCKED;
                queue_enqueue(blocked_queue, current_thread);
                uthread_yield();
            }
            else{
                break;
            }
        }

        if(retval != NULL) {
            *retval = current_thread->join->retval;
        }
        queue_delete(zombie_queue, current_thread->join);
        free_TCB_t(current_thread->join);
        current_thread->join = NULL;
    }
    preempt_enable();
    return SUCCEED;
}

TCB_t make_new_TCB(){
    //if TID overflow
    if(tid_overflow()){
        return NULL;
    }
    TCB_t  new_tcb_t = (TCB_t)malloc(sizeof(struct TCB));
    new_tcb_t->tid = current_max_tid++;
    new_tcb_t->state = UNINITIALIZED;
    new_tcb_t->joined_by = NULL;
    new_tcb_t->join = NULL;
    new_tcb_t->retval = 0;
    new_tcb_t->sp = uthread_ctx_alloc_stack();
    if(new_tcb_t->sp == NULL){
        return NULL;
    }
    new_tcb_t->context = (uthread_ctx_t*)malloc(sizeof(uthread_ctx_t));
    if(new_tcb_t->context == NULL){
        return NULL;
    }
    return new_tcb_t;
}

int tid_overflow(){
    int flag;
    preempt_disable();
    flag = current_max_tid >= USHRT_MAX;
    preempt_enable();
    return flag;
}

int init_context(TCB_t tcb_t, uthread_func_t func){
    return uthread_ctx_init(tcb_t->context, tcb_t->sp,func);
}

void free_TCB_t(TCB_t tcb_t){
    if(tcb_t->joined_by != NULL){
        tcb_t->joined_by = NULL;
    }
    if(tcb_t->join != NULL){
        tcb_t->join = NULL;
    }
    if(tcb_t->sp != NULL){
        free(tcb_t->sp);
    }
    if(tcb_t->context != NULL){
        free(tcb_t->context);
    }
}

int find_TCB_by_tid(uthread_t tid,  TCB_t* tcb_t_t){
    if(queue_iterate(ready_queue, find_TCB_t, &tid, (void**)tcb_t_t) == SUCCEED && *tcb_t_t != NULL){
        return SUCCEED;
    }

    if(queue_iterate(blocked_queue, find_TCB_t, &tid, (void**)tcb_t_t) == SUCCEED && *tcb_t_t != NULL){
        return SUCCEED;
    }

    if(queue_iterate(zombie_queue, find_TCB_t, &tid, (void**)tcb_t_t) == SUCCEED && *tcb_t_t != NULL){
        return SUCCEED;
    }

    return FAIL;
}

int find_TCB_t(queue_t q, void *data, void *arg){
    if(q == NULL){
        return 0;
    }
    TCB_t cur = (TCB_t)data;
    if(cur->tid == *(uthread_t*)arg){
        return 1;
    }
    else {
        return 0;
    }
}