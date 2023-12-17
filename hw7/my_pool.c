#include "my_pool.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void *tpool_thread(void *arg) {
  // TODO
  tpool *pool = (tpool *)arg;
  while (true) {
    pthread_mutex_lock(&pool->m_pool);
    while (!pool->end && pool->task_count == pool->n_tasks) {
      pthread_cond_wait(&pool->c_pool, &pool->m_pool);
    }
    if (pool->end && pool->task_count == pool->n_tasks) {
      pthread_mutex_unlock(&pool->m_pool);
      break;
    }
    task t = pool->tasks[pool->task_count];
    pool->task_count++;
    if (pool->end && pool->task_count == pool->n_tasks) {
      pthread_cond_broadcast(&pool->c_pool);
    } 
    pthread_mutex_unlock(&pool->m_pool);
    t.func(t.arg);
  }
  return NULL;
}

void tpool_add(tpool *pool, void *(*func)(void *), void *arg) {
  // TODO
  pthread_mutex_lock(&pool->m_pool);
  pool -> n_tasks++;
  pool -> tasks = (task *)realloc(pool -> tasks, (pool -> n_tasks) * sizeof(task));
  pool -> tasks[pool -> n_tasks - 1].func = func;
  pool -> tasks[pool -> n_tasks - 1].arg = arg;
  pthread_cond_signal(&pool->c_pool);
  pthread_mutex_unlock(&pool->m_pool);
}

void tpool_wait(tpool *pool) {
  // TODO
  pthread_mutex_lock(&pool->m_pool);
  pool->end = true;
  pthread_cond_broadcast(&pool->c_pool);
  pthread_mutex_unlock(&pool->m_pool);
  for (int i = 0; i < pool -> n_threads; i++) {
    pthread_join(pool -> threads[i], NULL);
  }
}

void tpool_destroy(tpool *pool) {
  // TODO
  pthread_mutex_destroy(&pool->m_pool);
  pthread_cond_destroy(&pool->c_pool);
  free(pool -> threads);
  free(pool -> tasks);
  free(pool);
}

tpool *tpool_init(int n_threads) {
  // TODO
  tpool *pool = (tpool *)malloc(sizeof(tpool));
  pool->n_threads = n_threads;
  pool->n_tasks = 0;
  pool->task_count = 0;
  pool->tasks = NULL;
  pool->threads = (pthread_t *)malloc(n_threads * sizeof(pthread_t));
  pthread_mutex_init(&pool->m_pool, NULL);
  pthread_cond_init(&pool->c_pool, NULL);
  pool->end = false;
  for (int i = 0; i < n_threads; i++) {
    pthread_create(&pool->threads[i], NULL, tpool_thread, (void *)pool);
  }
  return pool;
}