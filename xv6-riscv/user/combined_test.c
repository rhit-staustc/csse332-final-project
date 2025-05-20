// user/combined_test.c
// Demonstrate that two xv6 threads share the same user-level
// address space: one thread writes a global variable, the other
// spins until it observes the new value.

#include "kernel/types.h"
#include "user/user.h"

static volatile int sharednum = 0;

// -------------------------------------------------------------
// writer: set sharednum to 14 then exit
// -------------------------------------------------------------
void
writer_thread(void *arg)
{
  (void)arg;
  printf("Writer: setting sharednum to 14\n");
  sharednum = 14;
  exit(0);
}

// -------------------------------------------------------------
// reader: spin until sharednum != 0, then print it
// -------------------------------------------------------------
void
reader_thread(void *arg)
{
  (void)arg;
  while (sharednum == 0)
    sleep(1);

  printf("Reader: observed sharednum = %d\n", sharednum);
  exit(0);
}

// -------------------------------------------------------------
int
main(void)
{
  printf("=== combined (shared memory) test ===\n");

  int tid_w = thread_create(writer_thread, 0);
  int tid_r = thread_create(reader_thread, 0);

  if (tid_w < 0 || tid_r < 0) {
    printf("thread_create failed\n");
    exit(1);
  }

  // join writer then reader (order doesn’t matter here)
  if (thread_join(tid_w) < 0) {
    printf("join on writer failed\n");
    exit(1);
  }
  if (thread_join(tid_r) < 0) {
    printf("join on reader failed\n");
    exit(1);
  }

  printf("Main: test completed successfully\n");
  exit(0);
}

