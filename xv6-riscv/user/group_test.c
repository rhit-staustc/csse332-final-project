
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
  struct ctx *c = (struct ctx*)arg;

  // wait until parent/child flips its ready flag
  while (*(c->flag) == 0) ;
  __sync_synchronize();

  c->r->tid = getpid();
  c->r->n   = getfamily(c->r->fam, MAXF);

  exit(0);
}

// spawn n threads, flip flag, join; store into rec[]
static void
run_family(int n, struct rec *rb, volatile int *flag)
{
  struct ctx  ctxs[n];
  int         tids[n];

  for (int i = 0; i < n; i++) {
    ctxs[i].r    = &rb[i];
    ctxs[i].flag = flag;
    tids[i]      = thread_create(thread_fn, &ctxs[i]);
    if (tids[i] < 0) {
      printf("thread_create %d failed\n", i);
      exit(1);
    }
  }

  // release all threads
  __sync_synchronize();
  *flag = 1;

  for (int i = 0; i < n; i++)
    thread_join(tids[i]);
}

static void
show(const char *tag, struct rec *r)
{
  if (r->n < 0) {
    printf("%s TID %d getfamily failed\n", tag, r->tid);
    return;
  }
  printf("%s TID %d family(%d):", tag, r->tid, r->n);
  for (int i = 0; i < r->n; i++)
    printf(" %d", r->fam[i]);
  printf("\n");
}

int
main(void)
{
  static struct rec recA[NA];
  static struct rec recB[NB];

  printf("=== multiple-family test ===\n");

  // ─── parent builds Family A ───
  printf("Parent: spawn %d threads (Family A)\n", NA);
  run_family(NA, recA, &readyA);

  for (int i = 0; i < NA; i++)
    show("Family A:", &recA[i]);

  // ─── fork; child builds Family B ───
  int pid = fork();
  if (pid < 0) {
    printf("fork failed\n");
    exit(1);
  }
  if (pid == 0) {                     // child
    printf("Child: spawn %d threads (Family B)\n", NB);
    run_family(NB, recB, &readyB);
    for (int i = 0; i < NB; i++)
      show("Family B:", &recB[i]);
    printf("Child done.\n");
    exit(0);
  }

  // parent waits for child
  wait(0);
  printf("Parent done.\n");
  exit(0);
}

