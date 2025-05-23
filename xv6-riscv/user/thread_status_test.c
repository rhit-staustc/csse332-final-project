#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE }; (from kernel/proc.h)
#define UNUSED    0
#define USED      1
#define SLEEPING  2
#define RUNNABLE  3
#define RUNNING   4
#define ZOMBIE    5

#define MAX_FAMILY_MEMBERS 10

// Thread function that sleeps for a while
void sleeping_thread_func(void *arg) {
    //printf("T_sleep (%d) starting, will sleep for 20 ticks.\n", getpid());
    sleep(30); // Sleep long enough for main to check status
    //printf("T_sleep (%d) exiting.\n", getpid());
    exit(0);
}

// Thread function that performs computation
void compute_thread_func(void *arg) {
    //printf("T_compute (%d) starting computation.\n", getpid());
    volatile int sum = 0; // volatile to prevent optimization
    for (long i = 0; i < 1000000000; i++) { // Increased loop for longer computation
        sum += i;
    }
    //printf("T_compute (%d) finished computation, sum: %d. Exiting.\n", getpid(), sum);
    exit(0);
}

// Thread function that exits immediately
void immediate_exit_thread_func(void *arg) {
    //printf("T_exit_early (%d) exiting immediately.\n", getpid());
    exit(0);
}

// Helper to convert status code to string
const char* get_status_string_local(int status_val) {
    switch (status_val) {
        case UNUSED: return "UNUSED";
        case USED: return "USED";
        case SLEEPING: return "SLEEPING";
        case RUNNABLE: return "RUNNABLE";
        case RUNNING: return "RUNNING";
        case ZOMBIE: return "ZOMBIE"; // Rarely seen by getstatus due to unlinking first
        default: return "UNKNOWN/UNLINKED (-1)";
    }
}

// Helper to print statuses of the three threads
void print_all_statuses(int tid_s, int tid_c, int tid_e) {
    int stat_s = getstatus(tid_s);
    int stat_c = getstatus(tid_c);
    int stat_e = getstatus(tid_e);
    printf("  T_sleep(%d):%s, T_compute(%d):%s, T_exit_early(%d):%s\n",
           tid_s, get_status_string_local(stat_s),
           tid_c, get_status_string_local(stat_c),
           tid_e, get_status_string_local(stat_e));
}

int main(int argc, char *argv[]) {
    int tid_sleep, tid_compute, tid_exit_early;
    int family_tids[MAX_FAMILY_MEMBERS];
    int num_family_members;

    printf("Thread Status Test Program (Main PID: %d)\n", getpid());

    tid_sleep = thread_create(sleeping_thread_func, 0);
    tid_compute = thread_create(compute_thread_func, 0);
    tid_exit_early = thread_create(immediate_exit_thread_func, 0);

    if (tid_sleep < 0 || tid_compute < 0 || tid_exit_early < 0) {
        printf("ERROR: Thread creation failed.\n");
        exit(-1);
    }
    printf("Main created threads: T_sleep(%d), T_compute(%d), T_exit_early(%d)\n", tid_sleep, tid_compute, tid_exit_early);

    sleep(1); // Very short sleep: T_exit_early likely gone. T_compute should be active. T_sleep is sleeping.

    printf("Status check 1 (after 1 ticks):\n");
    print_all_statuses(tid_sleep, tid_compute, tid_exit_early);
    // Expected: T_sleep:SLEEPING, T_compute:RUNNING/RUNNABLE, T_exit_early:UNKNOWN

    num_family_members = getfamily(family_tids, MAX_FAMILY_MEMBERS);
    printf("Family TIDs (count: %d): ", num_family_members);
    for (int i = 0; i < num_family_members; i++) {
        printf("%d ", family_tids[i]);
    }
    //printf(" (Note: main is 0, may not include already exited/unlinked threads)\n");

    sleep(10); // More time. T_compute might finish. T_sleep still sleeping.

    printf("Status check 2 (after 11 total ticks):\n");
    print_all_statuses(tid_sleep, tid_compute, tid_exit_early);
    // Expected: T_sleep:SLEEPING, T_compute:UNKNOWN (if done) or RUNNING/RUNNABLE, T_exit_early:UNKNOWN

    sleep(22); // More time. T_sleep should finish now or soon. T_compute definitely done.
    printf("Status check 3 (after 33 total ticks):\n");
    print_all_statuses(tid_sleep, tid_compute, tid_exit_early);
    // Expected: T_sleep:UNKNOWN (if done), T_compute:UNKNOWN, T_exit_early:UNKNOWN

    //printf("Joining threads...\n");
    //printf("Attempting to join T_sleep(%d)...\n", tid_sleep);
    thread_join(tid_sleep);
    //printf("Joined T_sleep(%d).\n", tid_sleep);

    //printf("Attempting to join T_compute(%d)...\n", tid_compute);
    thread_join(tid_compute); 
    //printf("Joined T_compute(%d).\n", tid_compute);

    //printf("Attempting to join T_exit_early(%d)...\n", tid_exit_early);
    thread_join(tid_exit_early);
    //printf("Joined T_exit_early(%d).\n", tid_exit_early);
    
    printf("All threads joined.\n");
    
    printf("Final status check (after join, all should be UNKNOWN/UNLINKED as they are reaped):\n");
    print_all_statuses(tid_sleep, tid_compute, tid_exit_early);

    //printf("Thread status test finished.\n");
    exit(0);
}
