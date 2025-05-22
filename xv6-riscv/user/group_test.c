
// user/group_test.c
//
// Two independent thread families with clean, serialized output.
//
// Family A (parent): 5 threads
// Family B (child):  3 threads
//
// Each thread copies its own getfamily() result into a record.
// The parent/child print the records after joining, so lines never
// interleave and every thread reports the full family list.

#include "kernel/types.h"
#include "user/user.h"

#define NA     5          // threads in family A
#define NB     3          // threads in family B
#define MAXF   16         // buffer for getfamily()

// global ready-flags
volatile int readyA = 0;
volatile int readyB = 0;

// per-thread result
struct rec {
  int tid;
  int n;
  int fam[MAXF];
};

// context we pass to each thread (record + ready-flag)
struct ctx {
  struct rec *r;
  volatile int *flag;
};

// ───────────────── thread entry ─────────────────
void
thread_fn(void *arg)
{
  sleep(10);
  printf("Thread %d: Hello, world!\n", *(int*)arg);

  exit(0);
}

int
main(void)
{
  printf("Group test: Creating one thread\n");

  int tid = thread_create(thread_fn, 0);
  printf("Created thread %d\n", tid);

  sleep(10);

  exit(0);


  
}

