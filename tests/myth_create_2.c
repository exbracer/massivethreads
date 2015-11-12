
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <myth.h>

typedef struct {
  long a;
  long b;
  long r;
} arg_t;
 
void * f(void * arg_) {
  arg_t * arg = arg_;
  long a = arg->a, b = arg->b;
  if (b - a == 1) {
    arg->r = a;
  } else {
    long c = (a + b) / 2;
    arg_t cargs[2] = { { a, c, 0 }, { c, b, 0 } };
    myth_thread_t tid = myth_create(f, cargs);
    f(cargs + 1);
    myth_join(tid, 0);
    arg->r = cargs[0].r + cargs[1].r;
  }
}

#define nthreads 100

int main(int argc, char ** argv) {
  arg_t arg[1] = { { 0, nthreads, 0 } };
  myth_thread_t tid = myth_create(f, arg);
  myth_join(tid, 0);
  if (arg->r == (nthreads - 1) * nthreads / 2) {
    printf("OK\n");
    return 0;
  } else {
    printf("NG\n");
    return 1;
  }
}

