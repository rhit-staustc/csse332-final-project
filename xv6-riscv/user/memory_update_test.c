#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "knife_threads.h"


int *p = (int *)0xdeadbeef;

void *thread_t1(void *arg) {
    printf("thread 1\n");
    p = (int *)sbrk(4096);
    p[0] = 3;
    p[1] = 2;

    printf("thread 1 done\n");
    knife_thread_exit();
    return 0;
}

void *thread_t2(void *arg) {
    sleep(50); //sleep while t1 allocates
    printf("thread 2\n");

    printf("accessing p\n");
    if(p == (int *)0xdeadbeef) {
        printf("P did not get updated\n");
        knife_thread_exit();
    }

    printf("accessing args\n");
    if(p[0] == 3 && p[1] == 2) {
        printf("Success! P got updated\n");
        knife_thread_exit();
    } else {
        printf("P got updated but the values are wrong\n");
        printf("p[0] = %d, p[1] = %d\n", p[0], p[1]);
        knife_thread_exit();
    }
    return 0;
}

void *thread_t3(void *arg) {
    printf("thread 3\n");
    char *mem = (char *)sbrk(2 * 4096); // Allocate 2 pages
    if (mem == (char *)-1) {
        printf("thread 3: sbrk alloc failed\n");
        knife_thread_exit();
        return 0;
    }
    printf("thread 3: allocated 2 pages at %p\n", mem);

    int *sbrk_ret = (int *)sbrk(-4096);
    if (sbrk_ret == (int *)-1) {
        printf("thread 3: sbrk dealloc failed\n");
    } else {
        printf("thread 3: sbrk dealloc returned %p\n", sbrk_ret);
    }

    printf("thread 3 done\n");
    knife_thread_exit();
    return 0;
}


int main(int argc, char *argv[]) {

    knife_thread_t tid1, tid2, tid3;
    printf("=== MEMORY UPDATE TEST ===\n");

    if(knife_thread_create(&tid1, (void (*)(void *))thread_t1, 0) < 0) {
        printf("Failed to create thread 1\n");
        exit(-1);
    }
    
    if(knife_thread_create(&tid2, (void (*)(void *))thread_t2, 0) < 0) {
        printf("Failed to create thread 2\n");
        exit(-1);
    }

    if(knife_thread_join(tid1) < 0) {
        printf("Failed to join thread 1\n");
        exit(-1);
    }
    
    if(knife_thread_join(tid2) < 0) {
        printf("Failed to join thread 2\n");
        exit(-1);
    }
    
    printf("Threads 1 and 2 joined, now testing removing memory\n");

    if(knife_thread_create(&tid3, (void (*)(void *))thread_t3, 0) < 0) {
        printf("Failed to create thread 3\n");
        exit(-1);
    }

    if(knife_thread_join(tid3) < 0) {
        printf("Failed to join thread 3\n");
        exit(-1);
    }


    printf("Success! All threads joined\n");
    exit(0);
}
