#include "kernel/types.h"
#include "user/user.h"


int sharednum = 0;

void thread_test(void *arg){
	printf("Thread setting num to 50!\n");
	sharednum=50;
	while(1){
		continue;
	}
}

void thread_test2(void *arg){
	sleep(15);
	printf("Shared num is %d!/n", sharednum);
	while(1){
		continue;
	}
}

int main(){
	printf("Testing shared mem starting\n");
	
	int tid=thread_create(thread_test, (void*)val);

	if(tid<0){
		printf("thread creation faild\n");
	}

	sleep(10);

	int tid2=thread_create(thread_test2, 0);
	if(tid2<0){
		printf("Test 2 failed\n");
	}

	sleep(50);

	printf("done\n");
	while(1){
		continue;
	}
}
