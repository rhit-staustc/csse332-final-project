#include "helpers.h"

#include <readline/readline.h>
#include <string.h>

const char *commands[] = {
  "infantry",
  "armored",
  "cavalry",
  "list",
  "fire",
  "support",
  "dezombify",
  "nuke",
  NULL
};

void red(void)
{
  printf("\033[1;31m");
}

void green(void)
{
  printf("\033[1;32m");
}

void yellow(void)
{
  printf("\033[1;33m");
}

void purple(void)
{
  printf("\033[1;35m");
}

void reset(void)
{
  printf("\033[0m");
}

char *
name_generator(const char *text, int state)
{
  static int list_index, len;
  const char *name;

  if(!state) {
    list_index = 0;
    len = strlen(text);
  }

  while((name = commands[list_index++])) {
    if(!strncmp(name, text, len)) {
      return strdup(name);
    }
  }

  return NULL;
}


char **
char_name_completion(const char *text, int start, int end)
{
  rl_attempted_completion_over = 1;
  return rl_completion_matches(text, name_generator);
}

