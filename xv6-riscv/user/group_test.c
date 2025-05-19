#include "kernel/types.h"
#include "user/user.h"
#include "kernel/syscall.h"

#define MAXFAM 16
#define SYS_getFamily 25

int getFamily(int *buf, int max) {
	return syscall(SYS_getFamily, buf, max);
}

void thread_fn(void *arg) {
	int mytid = *(int*)arg;
	int fam[MAXFAM];
	int n;

	sleep(10);

	n = getFamily(fam, MAXFAM);
	if(n < 0) {
		printf("tid %d: getfamily failed\n", mytid);
		exit(1);
	}

	printf("tid %d: getfamily has %d members:", mytid, n);
	for(int i = 0; i < n; i++) {
		printf(" %d", fam[i]);
	}
	printf("\n");

	exit(0);
}

int
main(void){
	printf("=== family-list test starting ==\n");

	int tids[5];
	for(int i = 0; i < 5; i++) {
		int *arg = malloc(sizeof(*arg));
		*arg = i;
		tids[i] = thread_create(thread_fn, arg);
		if(tids[i] < 0) {
			printf("thread create %d failed\n", i);
			exit(1);
		}
	}

	printf("waiting for join\n");
	for(int i = 0; i < 5; i++) {
		thread_join(tids[i]);
	}

	printf("=== done ===\n");
}

}
