#ifndef _PRETTY_PRINT_H
#define _PRETTY_PRINT_H

#include <stdio.h>

// change color to red
void
red(void);

// change color to green
void
green(void);

// change color to yellow
void
yellow(void);

// change color to purple
void
purple(void);

// reset color back
void
reset(void);

// use pr_log to print in green in the same way you would use printf
#define pr_log(...) do {    \
  green();                  \
  printf(__VA_ARGS__);      \
  reset();                  \
} while(0);

// use pr_warn to print in yellow in the same way you would use printf
#define pr_warn(...) do {   \
  yellow();                 \
  printf(__VA_ARGS__);      \
  reset();                  \
} while(0);

// use pr_error to print in red in the same way you would use printf
#define pr_error(...) do {        \
  red();                          \
  printf(__VA_ARGS__);            \
  reset();                        \
} while(0);

#endif /* pretty_print.h */
