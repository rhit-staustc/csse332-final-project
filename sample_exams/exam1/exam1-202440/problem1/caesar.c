#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "helpers.h"

#define BUFSIZE 256

#define STUDENT_KEY 8

enum operation_mode {
  ENCRYPT = 0,
  DECRYPT = 1,

  last_one
};

char *caesar(const char *op, const char *message) {
  const char *p = message;
  char *result = malloc(strlen(message) + 1);
  char *q = result;
  enum operation_mode mode = (!strncmp(op, "encrypt", strlen(op)))? ENCRYPT : DECRYPT;

  while(*p) {
    if(isalpha(*p)) {
      switch(mode) {
        case ENCRYPT:
          if('a' <= *p && *p <= 'z') {
            *q = *p + STUDENT_KEY;
            if(*q > 'z') {
              *q = *q + 'a' - 'z' - 1;
            }
          } else if('A' <= *p && *p <= 'Z') {
            *q = *p + STUDENT_KEY;
            if(*q > 'Z') {
              *q = *q + 'A' - 'Z' - 1;
            }
          }
          break;
        default:
        case DECRYPT:
          if('a' <= *p && *p <= 'z') {
            *q = *p - STUDENT_KEY;
            if(*q < 'a') {
              *q = *q - 'a' + 'z' + 1;
            }
          } else if('A' <= *p && *p <= 'Z') {
            *q = *p - STUDENT_KEY;
            if(*q < 'A') {
              *q = *q - 'A' + 'Z' + 1;
            }
          }
          break;
      }
    } else {
      *q = *p;
    }

    p++;
    q++;
  }
  *q = 0;
  return result;
}

int
main(int argc, char **argv) {
  // arguments are one of these possibilities:
  //  1. encrypt "message"
  //  2. decrypt "message"
  //  3. encrypt/decrypt readfd
  //  4. encrypt/decrypt readfd writefd
  char *op = 0, *p = 0, *result = 0;
  int readfd = -1, writefd = -1;
  char buff[BUFSIZE];

  // parse the command line arguments
  if(argc < 2) {
    pr_error("You do not know how to call caesar.bin!\n");
    exit(EXIT_FAILURE);
  }

  op = argv[1]; // operation is always argv[1]
  if(strncmp(op, "encrypt", strlen(op)) && strncmp(op, "decrypt", strlen(op))) {
    pr_error("Invalid operation passed to caesar.bin: %s\n", op);
    exit(EXIT_FAILURE);
  }

  // check which mode we are going to do
  if(argc == 3) {
    // either op message or op readfd
    readfd = strtol(argv[2], &p, 10);
    if(*p) {
      // argv[2] is a message, so work on that assumption
      pr_warn("Running caesar cipher in %s mode on argument message!\n", op);
      result = caesar(op, argv[2]);
    } else {
      // argv[2] is indeed a readfd
      pr_warn("Running caesar cipher in %s mode with readfd = %d!\n", op, readfd);
      read(readfd, buff, BUFSIZE);
      close(readfd);

      result = caesar(op, buff);
    }

    pr_log("[%s:%d] Output of %s operation: %s\n", argv[0], getpid(), op, result);
    free(result);
    exit(EXIT_SUCCESS);
  } else if (argc == 4) {
    // op readfd writefd
    readfd = strtol(argv[2], &p, 10);
    if(*p) {
      pr_error("Invalid call to caesar.bin, plese re-read the instructions!\n");
      exit(EXIT_FAILURE);
    }
    writefd = strtol(argv[3], &p, 10);
    if(*p) {
      pr_error("Invalid call to caesar.bin, plese re-read the instructions!\n");
      exit(EXIT_FAILURE);
    }

    pr_warn("Running ceasar cipher in %s mode with readfd = %d and writefd = %d\n",
            op, readfd, writefd);

    // read the message from the user: assume everything is shorter than
    // BUFSIZE
    read(readfd, buff, BUFSIZE);
    close(readfd);

    result = caesar(op, buff);

    // computed result, dump it into the write fd
    write(writefd, result, strlen(result)+1);
    close(writefd);

    // done, free up and leave
    free(result);
    exit(EXIT_SUCCESS);
  } else {
    // ??
    pr_error("Unknow call to caesar.bin, please re-read the instructions!\n");
    exit(EXIT_FAILURE);
  }
}
