#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "helpers.h"

/**
 * Feel free to use this function if needed, it is the same one as we used in
 * class.
 */
void setsighandler(int signum, void (*handler)(int)) {
  struct sigaction act;

  act.sa_handler = handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_RESTART;
  sigaction(signum, &act, NULL);
}

/**
 * handle the infantry command
 */
void
infantry() {
  // TODO: Add your code for the infantry command here...
}

volatile int hit_count = 0;

/**
 * handle the cavalry command
 */
void
cavalry() {
  // TODO: Add your code for the cavalry command here...
}

/**
 * handle the armored command
 */
void
armored() {
  // TODO: Add your code for the armored command here...
}

/**
 * handle the fire command with a given pid
 */
void
fire(int pid) {
  // TODO: Add your code for the fire command here...
}

/**
 * handle the list command
 */
void
list() {
  // TODO: Add your code for the list command here...
}

/**
 * handle the dezombify command with a given pid
 */
void
dezombify(int pid) {
  // TODO: Add your code for the dezombify command here...
}

/*
 * handle the support command with a given pid
 */
void
support(int pid) {
  // TODO: Add your code for the support command here...
}

/*
 * handle the nuke command
 */
void
nuke() {
  // TODO: Add your code for the nuke command here...
}

