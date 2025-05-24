#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "knife_threads.h"

void thread_fn(void *arg)
{
    free(arg); 
    knife_thread_exit();
}

// In main:
int main(void)
{
    printf("==GROUP TEST===\n");

    int *arg = malloc(sizeof(int));
    *arg = 58;

    knife_thread_t tid;
    if (knife_thread_create(&tid, thread_fn, arg) < 0) {
        printf("Failed to create thread\n");
        exit(-1);
    }
    printf("Created thread %d\n", tid);


    int *arg2 = malloc(sizeof(int));
    *arg2 = 59;

    knife_thread_t tid2;
    if (knife_thread_create(&tid2, thread_fn, arg2) < 0) {
        printf("Failed to create thread\n");
        exit(-1);
    }
    printf("Created thread %d\n", tid2);

    // Wait specifically for our thread to finish
    if (knife_thread_join(tid) < 0) {
        printf("Failed to join thread\n");
        exit(-1);
    }

    printf("Thread %d joined successfully\n", tid);


    if (knife_thread_join(tid2) < 0) {
        printf("Failed to join thread\n");
        exit(-1);
    }

    printf("Thread %d joined successfully\n", tid2);
    exit(0);
}
