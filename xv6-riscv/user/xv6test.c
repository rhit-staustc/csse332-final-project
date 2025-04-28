#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void test(void *arg){
	printf("Running test thtead");
}

int main(int argc, char *argv[]) {
  printf("Hello from my test case in xv6\n");

  //uint64 p = 0xdeadbeef;
  //spoon((void*)p);

  thread_create(test, 0);
  thread_join(0);
  exit(0);
}
