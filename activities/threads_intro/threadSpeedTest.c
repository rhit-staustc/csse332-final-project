#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>

#define WORK_SIZE 20000

// Many thanks to Micah for the initial starting code (which I have modified
// significantly)

/* This code runs some long running calculations using various types
   of parallelism.  You can adjust the work size above to change the
   amount of work done.

   First, write code that would split the calculation betwen two processes.

   Then, write code that would split the calculation between two threads.

   Run the binary and check the differences.

   Check out the solution directory for scaling up to more and more threads and processes.

   */

#define THREAD_COUNT 5

float* dest;

void output_time_difference(char* name, struct timeval* start,
			    struct timeval* end) {
	long secs_used =
		(end->tv_sec - start->tv_sec);  // avoid overflow by subtracting first
	long usecs_used = (end->tv_usec - start->tv_usec);
	double secs = secs_used + (double)usecs_used / 1000000;
	printf("%s took %f seconds\n", name, secs);
}

// naive exponentiation is useful because it requires a lot of compute
int power(int a, int b) {
	int i;
	int r = a;
	for (i = 1; i < b; i++) r = r * a;
	return r;
}

void* threadFun(void* startingInt) {
	int start = *((int*)startingInt);
	for (int i = start; i < WORK_SIZE; i += 2) {
		dest[i] = power(i, i);
	}
}

int main(int argc, char** argv) {
	struct timeval start, end;
	int i;
	dest = (float*)malloc(sizeof(float) * WORK_SIZE);

	// PART 1: ------------------------no paralellism

	gettimeofday(&start, NULL);
	for (i = 0; i < WORK_SIZE; i++) {
		dest[i] = power(i, i);
	}
	gettimeofday(&end, NULL);
	output_time_difference("simple for loop", &start, &end);

	// PART 2: ------------------------use fork

	gettimeofday(&start, NULL);

  // mmap allows us to share an area of memory between processes
  // you don't have to worry about this for today.
	dest = mmap(NULL, sizeof(float) * WORK_SIZE, PROT_READ | PROT_WRITE,
		    MAP_SHARED | MAP_ANONYMOUS, -1, 0);

  // TODO: Add your code here...

	gettimeofday(&end, NULL);
	output_time_difference("fork", &start, &end);

	// PART 3: ------------------------use pthreads

	gettimeofday(&start, NULL);

  // TODO: Add your code here.....

	gettimeofday(&end, NULL);

	output_time_difference("pthreads", &start, &end);

	return 0;
}
