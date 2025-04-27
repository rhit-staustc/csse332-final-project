#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  printf("Hello from my test case in xv6\n");

  uint64 p = 0xdeadbeef;
  spoon((void*)p);

  exit(0);
}
