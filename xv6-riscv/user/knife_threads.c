#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "user.h"
#include "knife_threads.h"

extern int thread_create(void (*)(void *), void *);
extern int thread_join(int);

int knife_thread_create(knife_thread_t *knife_tid,
                        void (*thread_fn)(void *), 
                        void *arg) {
  int tid = thread_create(thread_fn, arg);
  if (tid < 0) {
    return -1;
  }
  if (tid) {
    *knife_tid = tid;
  }
  return 0;
}

int knife_thread_join(knife_thread_t tid) {
  return thread_join(tid);
}

void knife_thread_exit(void) {
  thread_exit();
  // If thread_exit returns, something went wrong
  printf("thread_exit returned! This should never happen.\n");
  exit(-1);
}

int knife_getfamily(int *buf, int max) {
  return getfamily(buf, max);
}

int knife_getstatus(int tid) {
  return getstatus(tid);
}


