#include "kernel/types.h"
#include "user/user.h"
#include "user/knife_threads.h"

/*
 Tests functionality of creating a thread with an argument.
 Creates two threads, one with an argument and one without.
 Each thread prints a message and either sleeps or loops. 

 knife_thread_create() is used to create the threads.
 knife_thread_exit() is called to exit the thread.
*/


void thread_test(void *arg){
	int num = *(int *) arg;
	printf("Thread 1 created with arg %d, sleeping now\n", num);
	sleep(10);
	printf("Thread 1 done, exiting\n");
	knife_thread_exit();
}

void thread_test2(void *arg){
	printf("Thread 2 created: looping\n");
	for(int i = 0; i<3;i++){
		printf("Thread 2 iteration %d\n", i);
		sleep(5);
	}
	printf("Thread 2 done, exiting\n");
	knife_thread_exit();
}

void main_thread_createtest_impl(void){
	printf("=== Thread Create Test ===\n");
	printf("Will create two threads, one with an argument and one without.\n");
	printf("The test will pass if the threads exit successfully.\n");
	printf("This test will expectently leak memory.\n");
	printf("\n");
	int *val = malloc(sizeof(int));

	*val = 91;
	int *tid = malloc(sizeof(int));
	if(knife_thread_create(tid, thread_test, (void*)val) < 0){
		printf("Test 1 failed\n");
		exit(1);
	}

	sleep(50);

	int *tid2 = malloc(sizeof(int));
	if(knife_thread_create(tid2, thread_test2, 0) < 0){
		printf("Test 2 failed\n");
		exit(1);
	}

	sleep(50);

	printf("=== Thread Create Test Finished Successfully ===\n");
	exit(1);
}
