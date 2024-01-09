#include "my_pool.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
typedef long long LL;
void *Collatz(void *args) {
  LL x = *(LL *)args;
  LL cnt = 0;
  while (x != 1) {
    if (x & 1)
      x = x * 3 + 1;
    else
      x /= 2;
    cnt++;
    // try uncomment printf
    // printf("%lld\n", x);
  }
  // try uncomment printf
  // printf("%lld\n", cnt);
  return NULL;
}
#define N 2
#define M 0x2
int main() {
  tpool *pool1 = tpool_init(N);
  tpool *pool2 = tpool_init(N);
  pool1 -> index = 1;
  pool2 -> index = 2;
  LL *arg = malloc(M * sizeof(LL));
  for (int i = 0; i < M; i++) {
    arg[i] = 0x10000000ll + i;
    tpool_add(pool1, Collatz, (void *)&arg[i]);
    tpool_add(pool2, Collatz, (void *)&arg[i]);
  }
  tpool_wait(pool1);
  printf("pool1 done\n");
  tpool_wait(pool2);
  printf("pool2 done\n");
  tpool_destroy(pool1);
  tpool_destroy(pool2);
  free(arg);
}