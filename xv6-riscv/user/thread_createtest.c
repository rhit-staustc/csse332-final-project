#include "kernel/types.h"
#include "user/user.h"

void thread_test(void *arg){
	int num = *(int *) arg;
	printf("Thread created wit num %d\n", num);
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

	printf("done\n");
	exit(1);
}
