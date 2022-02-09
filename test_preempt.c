#include <stdio.h>

#include "uthread.h"


char* str = "This only prints with preempt\n";

int print_thread(void) {
    printf("in thread2: tid= %u\n", uthread_self());
    return 0;
}

int infinite_thread(void) {
    while(1);
    return 0;
}


int main() {
    uthread_start(1);
    uthread_create(infinite_thread);
    uthread_t idt = uthread_create(print_thread);
    uthread_join(idt, NULL);
    printf("return from join in main\n");
    uthread_stop();
    return 0;
}