#include "threadtools.h"
#include <sys/signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * Print out the signal you received.
 * If SIGALRM is received, reset the alarm here.
 * This function should not return. Instead, call siglongjmp(sched_buf, 1).
 */
void sighandler(int signo) {
    // TODO
    sigprocmask(SIG_SETMASK, &base_mask, NULL);
    if (signo == SIGALRM) {
        printf("caught SIGALRM\n");
        alarm(timeslice);
    } else if (signo == SIGTSTP) {
        printf("^Zcaught SIGTSTP\n");
    }
    longjmp(sched_buf, 1);
}

void val1() {
    if (bank.lock_owner == -1 && wq_size > 0) {
        ready_queue[rq_size] = waiting_queue[0];
        rq_size++;
        wq_size--;
        for (int i = 0; i < wq_size; i++) {
            waiting_queue[i] = waiting_queue[i + 1];
        }
    }
    rq_current = (rq_current + 1) % rq_size;
}

void val2() {
    waiting_queue[wq_size] = ready_queue[rq_current];
    wq_size++;
    rq_size--;
    if (rq_current == rq_size) {
        rq_current = 0;
    } else {
        ready_queue[rq_current] = ready_queue[rq_size];
    }
}

void val3() {
    if (bank.lock_owner == -1 && wq_size > 0) {
        ready_queue[rq_size] = waiting_queue[0];
        rq_size++;
        wq_size--;
        for (int i = 0; i < wq_size; i++) {
            waiting_queue[i] = waiting_queue[i + 1];
        }
    }
    rq_size--;
    if (rq_size <= 0) return;
    if (rq_current == rq_size) {
        rq_current = 0;
    } else {
        ready_queue[rq_current] = ready_queue[rq_size];
    }
}

/*
 * Prior to calling this function, both SIGTSTP and SIGALRM should be blocked.
 */
void scheduler() {
    // TODO
    if (rq_size == 0) {
        return;
    }
    rq_current = 0;
    setjmp(sched_buf);
    int val = setjmp(sched_buf);
    switch (val) {
        case 1:
            val1();
            break;
        case 2:
            val2();
            break;
        case 3:
            val3();
            break;
    }
    if (rq_size == 0) {
        return;
    }
    longjmp(ready_queue[rq_current]->environment, 1);
}

    