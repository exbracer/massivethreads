/*
 * test program for the task_group class (identical to
 * the one found in TBB) on top of 
 * MassiveThreads/Qthreads/NANOX/TBB
 */

#include <stdio.h>

extern "C" {
#if TO_SERIAL
  int dr_get_worker() { return 0; }
  int dr_get_cpu() { return 0; }
  int dr_get_num_workers() { return 1; }
#else  /* with MassiveThreads */
#include <myth.h>
  int sched_getcpu();
  int dr_get_worker() { return myth_get_worker_num(); }
  int dr_get_cpu() { return sched_getcpu(); }
  int dr_get_num_workers() { return myth_get_num_workers(); }
#endif
}

#define DAG_RECORDER 2
#include <mtbb/task_group.h>

long bin(int n);

struct bin_ {
  long &x;
  int n;
  bin_(long &x_, int n_) : x(x_), n(n_) { }
  void operator() () {
    x = bin(n);
  }
};

long bin(int n) {
  if (n == 0) {
    return 1;
  } else {
    mtbb::task_group tg;
    long x, y;
    tg.run(bin_(x, n - 1));
    y = bin(n - 1);
    tg.wait();
    return x + y;
  }
}

int main(int argc, char ** argv) {
  int n = (argc > 1 ? atoi(argv[1]) : 10);
  dr_options opts[1];
  dr_options_default(opts);
  opts->collapse = 0;
  dr_start(opts);
  long x = bin(n);
  dr_stop();
  printf("bin(%d) = %ld\n", n, x);
  //dr_print_task_graph(0);
  //dr_gen_dot_task_graph(0);
  return 0;
}

