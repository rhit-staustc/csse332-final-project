#include "kernel/types.h"
#include "user/user.h"

void thread_test(void *arg){
	int num = *(int *) arg;
	printf("Thread created wit num %d\n", num);
	sleep(10);
	printf("Thread done\n");
	exit(0);
}

void thread_test2(void *arg){
	for(int i = 0; i<3;i++){
		printf("Thread iteration %d\n", i);
		sleep(5);
	}
	exit(0);
}

int main(){
	printf("Testing starting\n");
	int *val = malloc(sizeof(int));

	*val = 91;
	int tid=thread_create(thread_test, (void*)val);

	if(tid<0){
		printf("thread creation faild\n");
	}

	sleep(50);

	int tid2=thread_create(thread_test2, 0);
	if(tid2<0){
		printf("Test 2 failed\n");
	}

	sleep(50);

	printf("done\n");
	exit(1);
}
