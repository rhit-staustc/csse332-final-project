#include <stdlib.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "helpers.h"

extern void infantry();
extern void armored_handler(int);
extern void cavalry();
extern void armored();
extern void fire(int);
extern void support(int);
extern void dezombify(int);
extern void list();
extern void nuke();

// forward declarations, DO NOT EDIT THESE LINES
char *get_line();
void parse_command(char*);

/*
 * main function
 */
int
main(int argc, char **argv) {
  char *line = 0; // the line read from the terminal window
  
  rl_attempted_completion_function = char_name_completion;
  
  while(1) {
    // grab the line from the user, empty lines are okay, but do nothing.
    line = get_line();

    // add to history so you can press up arrow key. DO NOT EDIT
    if(line && *line) {
      add_history(line);
    }

    parse_command(line);
    reset();
  }
}

// Get a line from the user using libreadline
char *get_line(void) {
  static char *line_read = 0;

  if(line_read) {
    free(line_read);
    line_read = 0;
  }

  line_read = readline("commander >> ");
  return line_read;
}

// parse the command passed in by the user and call the appropriate function.
void
parse_command(char *line) {
  char *cmd[2] = {0, 0};
  long target;

  // minor error checking. DO NOT EDIT
  if(!line || !(*line)) return;

  // check if we should quit.
  if(!strncmp(line, "quit", strlen("quit"))) {
    pr_log("Goodbye cruel world....\n");
    exit(EXIT_SUCCESS);
  } else if(!strncmp(line, "infantry", strlen("infantry"))) {
    infantry();
  } else if(!strncmp(line, "cavalry", strlen("cavalry"))) {
    cavalry();
  } else if(!strncmp(line, "armored", strlen("armored"))) {
    armored();
  } else if(!strncmp(line, "list", strlen("list"))) {
    list();
  } else if(!strncmp(line, "nuke", strlen("nuke"))) {
    nuke();
  } else {
    cmd[0] = strtok(line, " ");
    cmd[1] = strtok(0, " ");

    if(!strncmp(cmd[0], "fire", strlen("fire"))) {
      if(!cmd[1]) {
        pr_error("You do not know how to fire....\n");
        return;
      }
      target = strtol(cmd[1], 0, 10);
      if(!target) {
        pr_warn("Cannot shoot at blank (target = %ld)\n", target);
        return;
      }

      pr_warn("You shot %ld\n", target);
      fire(target);
    } else if(!strncmp(cmd[0], "support", strlen("support"))) {
      if(!cmd[1]) {
        pr_error("You do not know how to call air support....\n");
        return;
      }
      target = strtol(cmd[1], 0, 10);
      if(!target) {
        pr_warn("Cannot call air support at blank (target = %ld)\n", target);
        return;
      }

      pr_log("You called air support on %ld\n", target);
      support(target);
    } else if(!strncmp(cmd[0], "dezombify", strlen("dezombify"))) {
      if(!cmd[1]) {
        pr_error("You do not know how to call dezombification teams....\n");
        return;
      }
      target = strtol(cmd[1], 0, 10);
      if(!target) {
        pr_warn("Cannot send dezombification team at blank (target = %ld)\n", target);
        return;
      }

      pr_log("Sending dezombification team to target %ld\n", target);
      dezombify(target);
    } else 
      pr_error("Unknow command, please try again....\n");
  }
  // handle libreadline completions
}

