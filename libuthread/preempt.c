#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100

/**
 * sigvtalrm alarm
 */
sigset_t vt_alarm;
//for restoring itimerval in preempt_stop
struct itimerval old_timerval;
struct itimerval new_timerval;
//for restoring sigaction in preempt_stop
struct sigaction old_action;
struct sigaction new_action;

void sigvalrm_handler();
// indicate whether preempty mode is set
int preemption_set = 0;

void preempt_start(void)
{
    /* TODO */
    new_action.sa_handler = sigvalrm_handler;
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction (SIGVTALRM,&new_action, &old_action);

    new_timerval.it_value.tv_sec = 0;
    new_timerval.it_value.tv_usec = 1000000 / HZ;
    new_timerval.it_interval.tv_sec = 0;
    new_timerval.it_interval.tv_usec = 1000000 / HZ;
    setitimer(ITIMER_VIRTUAL, &new_timerval, &old_timerval);
    preemption_set = 1;
}

void preempt_stop(void)
{
    /* TODO */
    //only restore if preempt mode is previously set
    if(preemption_set){
        sigaction (SIGVTALRM,&old_action, NULL);
        setitimer(ITIMER_VIRTUAL, &old_timerval, NULL);
        preemption_set = 0;
    }
}

void preempt_enable(void)
{
    /* TODO */
    sigemptyset(&vt_alarm);
    sigaddset(&vt_alarm, SIGVTALRM);
    sigprocmask (SIG_UNBLOCK, &vt_alarm, NULL);
}

void preempt_disable(void)
{
    /* TODO */
    sigemptyset(&vt_alarm);
    sigaddset(&vt_alarm, SIGVTALRM);
    sigprocmask (SIG_BLOCK, &vt_alarm, NULL);
}

void sigvalrm_handler(){
    preempt_disable();
    uthread_yield();
}
