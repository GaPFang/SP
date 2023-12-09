#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include "threadtools.h"

int timeslice;
sigset_t base_mask, tstp_mask, alrm_mask;
struct tcb *ready_queue[], *waiting_queue[];
int rq_size, rq_current, wq_size;
jmp_buf sched_buf;
struct Bank bank;
struct timespec tim;


/* prototype of the thread functions */
void fibonacci(int, int);
void factorial(int, int);
void bank_operation(int, int);
#ifdef abc
void bank_operation2(int, int);
#endif
/*
 * This function turns stdin, stdout, and stderr into unbuffered I/O, so:
 *   - you see everything your program prints in case it crashes
 *   - the program behaves the same if its stdout doesn't connect to a terminal
 */
void unbuffered_io() {
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
}

/*
 * Initializes the signal masks and the signal handler.
 */
void init_signal() {
    /* initialize the signal masks */
    sigemptyset(&base_mask);
    sigaddset(&base_mask, SIGTSTP);
    sigaddset(&base_mask, SIGALRM);
    sigemptyset(&tstp_mask);
    sigaddset(&tstp_mask, SIGTSTP);
    sigemptyset(&alrm_mask);
    sigaddset(&alrm_mask, SIGALRM);

    /* initialize the signal handlers */
    signal(SIGTSTP, sighandler);
    signal(SIGALRM, sighandler);

    /* block both SIGTSTP and SIGALRM */
    sigprocmask(SIG_SETMASK, &base_mask, NULL);
}

int f[16], arg[16], n;

/*
 * Threads are created nowhere else but here.
 */
void init_threads() {
    char padding[64];
    for(int i = 0; i < n; ++i){
        if(f[i] == 0) thread_create(fibonacci, i, arg[i]);
        if(f[i] == 1) thread_create(factorial, i, arg[i]);
        if(f[i] == 2) thread_create(bank_operation, i, arg[i]);
#ifdef abc
        if(f[i] == 3) thread_create(bank_operation2, i, arg[i]);
#endif
    }
}

/* 
 * Calls the scheduler to begin threading.
 */
void start_threading() {
    alarm(timeslice);
    scheduler();
}


int main(int argc, char *argv[]) {
    n = (argc - 3) / 2;
    timeslice = atoi(argv[1]);
    for(int i = 0; i < n; ++i){
        f[i]   = atoi(argv[((i + 1) << 1)]);
        arg[i] = atoi(argv[((i + 1) << 1) + 1]);
    }
    unbuffered_io();
    init_signal();
    bank.balance = atoi(argv[argc - 1]);
    if(bank.balance < 0) bank.balance *= -1;
    bank.lock_owner = -1;
    init_threads();
    start_threading();
}
