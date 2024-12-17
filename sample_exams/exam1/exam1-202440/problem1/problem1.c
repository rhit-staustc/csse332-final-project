#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include "helpers.h"

void step1(const char *op, const char *message) {
  pr_error("Step 1 not implemented yet!\n");
}

void step2(const char *op, const char *message) {
  pr_error("Step 2 not implemented yet!\n");
}

void step3(const char *op, const char *message) {
  pr_error("Step 3 not implemented yet!\n");
}

int
main(int argc, char **argv) {
  char *message, *op;
  if(argc < 3) {
    pr_error("You did not invoke problem1.bin correctly!\n");
    exit(EXIT_FAILURE);
  }
  op = argv[1];
  message = argv[2];

  pr_log("Parent running step 1\n");
  step1(op, message);
  printf("\n\n");

  pr_log("Parent running step 2\n");
  step2(op, message);
  printf("\n\n");

  pr_log("Parent running step 3\n");
  step3(op, message);
  printf("\n\n");

  exit(EXIT_SUCCESS);
}
