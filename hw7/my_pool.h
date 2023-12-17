#include <pthread.h>
#include <stdbool.h>
#ifndef __MY_THREAD_POOL_H
#define __MY_THREAD_POOL_H

typedef struct task {
  void *(*func)(void *);
  void *arg;
} task;

typedef struct tpool {
  // TODO: define your structure
  int n_threads, n_tasks;
  pthread_t *threads;
  task *tasks;
  int task_count;
  pthread_mutex_t m_pool;
  pthread_cond_t c_pool;
  bool end;
} tpool;

tpool *tpool_init(int n_threads);
void tpool_add(tpool *, void *(*func)(void *), void *);
void tpool_wait(tpool *);
void tpool_destroy(tpool *);

#endif