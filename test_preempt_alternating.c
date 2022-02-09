#include <stdio.h>

#include "uthread.h"


char* str = "This only prints with preempt\n";

int thread2(void) {
    while(1) {
        printf("in thread2: tid= %u\n", uthread_self());
    }
    return 0;
}

int thread1(void) {
    while(1){
        printf("in thread1: tid= %u\n", uthread_self());
    }
    return 0;
}

void test_preempt() {
    int id1 = uthread_create(thread1);
    int id2 = uthread_create(thread2);
    uthread_join(id1, NULL);
    printf("after join thread1\n");
    uthread_join(id2, NULL);
}

int main() {
    uthread_start(1);
    test_preempt();
    uthread_stop();
    return 0;
}