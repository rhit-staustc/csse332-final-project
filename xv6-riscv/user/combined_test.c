#include "kernel/types.h"
#include "user/user.h"

int sharednum = 0;

void writer_thread(void *arg){
	printf("Writer setting num to 14\n");
	sharednum=14;
	exit(0);
}

void reader_thread(void *arg){
	while(sharednum!=14){
		sleep(1);
	}
	printf("Reader reading num as %d\n", sharednum);
	exit(0);
}

int main(){
	printf("Main: writing!\n");
	int tid1=thread_create(writer_thread,0);


	printf("Main: reading!\n");
	int tid2=thread_create(reader_thread,0);

	if(tid1 < 0 || tid2 < 0){
		printf("thread creation failed\n");
		exit(1);
	}

	if(thread_join(tid1)<0){
		printf("writer failed\n");
		exit(1);
	}


	if(thread_join(tid2)<0){
		printf("reader failed\n");
		exit(1);
	}

	printf("Main ocmplted succesfullt\n");
	exit(0);
}
