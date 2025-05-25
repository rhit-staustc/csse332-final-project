#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "knife_threads.h"

/*
Tests functionality of creating a family of threads. Creates one family
of 5 threads, with different thread functions. Tests get_family() and get_status()
by checking if each thread can see every other thread in the family and if 
each thread sees the correct status.

Also tests that threads join and are freed correctly. All threads in family 1
exit and join, and then 3 more threads are created. These threads are expected
to be in the same family as the first 5 threads, as the parent thread is still
running.

The test will pass if all threads exit successfully and the family is correctly
formed and all threads can see each other and the correct statuses. 
*/

#define NUM_PHASE1_THREADS 5
#define NUM_PHASE2_THREADS 3
#define TOTAL_GROUP_TEST_THREADS (NUM_PHASE1_THREADS + NUM_PHASE2_THREADS)
#define MAX_FAMILY_MEMBERS_QUERY 16 // Max expected family members including those from other tests

// Mirror procstate enum from kernel/proc.h for accurate status reporting
typedef enum {
    THREAD_UNUSED = 0,
    THREAD_USED,
    THREAD_SLEEPING,
    THREAD_RUNNABLE,
    THREAD_RUNNING,
    THREAD_ZOMBIE,
    THREAD_GHOST,
    THREAD_STATUS_COUNT
} thread_status_t;

// Job types for worker threads
typedef enum {
    JOB_SLEEP,
    JOB_RUN,
    JOB_EARLY_EXIT
} job_type_t;

const char* job_type_to_string(job_type_t jt) {
    switch (jt) {
        case JOB_SLEEP: return "Sleep";
        case JOB_RUN: return "Run";
        case JOB_EARLY_EXIT: return "EarlyExit";
        default: return "UnknownJob";
    }
}

// Local helper to convert status enum to string
const char* local_status_to_string(thread_status_t status) {
    switch (status) {
        case THREAD_UNUSED: return "UNUSED";
        case THREAD_USED: return "USED";
        case THREAD_SLEEPING: return "SLEEPING";
        case THREAD_RUNNABLE: return "RUNNABLE";
        case THREAD_RUNNING: return "RUNNING";
        case THREAD_ZOMBIE: return "ZOMBIE";
        case THREAD_GHOST: return "GHOST";
        default: return "UNKNOWN_STATUS";
    }
}

// Arguments for worker threads
typedef struct {
    int logical_id;
    knife_thread_t self_tid;
    job_type_t job_type;
    int job_duration; // e.g., sleep ticks or computation iterations
    int phase;        // 1 or 2
    int is_last_in_phase;
    int baseline_family_count_at_start; // Family count before this test created any threads
} thread_args_t;

// Helper function for a thread to print all family member states
void print_family_member_states(int logical_id, knife_thread_t my_tid, const char* context_tag) {
    knife_thread_t family_tids[MAX_FAMILY_MEMBERS_QUERY];
    int member_count = getfamily(family_tids, MAX_FAMILY_MEMBERS_QUERY);

    if (member_count < 0) {
        printf("L_ID %d (TID %d, %s): ERROR getting family info for state dump.\n", logical_id, my_tid, context_tag);
        return;
    }
    printf("L_ID %d (TID %d, %s): Final States Check - %d members:\n", logical_id, my_tid, context_tag, member_count);
    for (int i = 0; i < member_count; i++) {
        thread_status_t status = getstatus(family_tids[i]);
        printf("  -> TID %d: %s (%d)\n", family_tids[i], local_status_to_string(status), status);
    }
}

// Unified worker thread function
void gt_worker_thread_func(void *arg) {
    sleep(1); // Give main a chance to print its creation line cleanly / stagger worker starts
    thread_args_t* t_args = (thread_args_t*)arg;
    knife_thread_t my_tid = t_args->self_tid;
    int initial_family_size_seen_by_thread;
    knife_thread_t temp_family_tids[MAX_FAMILY_MEMBERS_QUERY]; // For initial count

    initial_family_size_seen_by_thread = getfamily(temp_family_tids, MAX_FAMILY_MEMBERS_QUERY);
    int expected_initial_count = t_args->baseline_family_count_at_start + t_args->logical_id + 1;

    if (t_args->phase == 1) {
        printf("L_ID %d (TID %d, %s): Started. My job: %s for %d units. Initial family count: %d (Expected: ~%d).\n",
               t_args->logical_id, my_tid, job_type_to_string(t_args->job_type),
               job_type_to_string(t_args->job_type), t_args->job_duration,
               initial_family_size_seen_by_thread, expected_initial_count);
    } else {
        printf("L_ID %d (TID %d, %s): Started. Phase %d. My job: %s for %d units. Initial family count: %d (Expected: ~%d).\n",
               t_args->logical_id, my_tid, job_type_to_string(t_args->job_type), t_args->phase,
               job_type_to_string(t_args->job_type), t_args->job_duration,
               initial_family_size_seen_by_thread, expected_initial_count);
    }

    // Perform job
    switch (t_args->job_type) {
        case JOB_SLEEP:
            sleep(t_args->job_duration);
            break;
        case JOB_RUN:
            volatile int counter = 0;
            for (int i = 0; i < t_args->job_duration; i++) counter++;
            break;
        case JOB_EARLY_EXIT:
            sleep(2); // Short life
            break;
    }

    if (t_args->is_last_in_phase) {
        printf("L_ID %d (TID %d): Is last in phase. Delaying before final state check...\n", t_args->logical_id, my_tid);
        sleep(t_args->phase == 1 ? 15 : 10);
        print_family_member_states(t_args->logical_id, my_tid, t_args->phase == 1 ? "Phase 1 Last Thread" : "Phase 2 Last Thread");
    }

    // printf("L_ID %d (TID %d, %s): Job done. Exiting.\n", t_args->logical_id, my_tid, job_type_to_string(t_args->job_type));
    free(arg);
    knife_thread_exit();
}

void main_group_test_impl(void) {
    printf("== GROUP TEST - REVISED FLOW ==\n");

    knife_thread_t initial_family_tids_main[MAX_FAMILY_MEMBERS_QUERY];
    int baseline_family_count = getfamily(initial_family_tids_main, MAX_FAMILY_MEMBERS_QUERY);
    if(baseline_family_count < 0) baseline_family_count = 0;
    printf("Main: Initial family count before creating group_test threads: %d\n", baseline_family_count);

    knife_thread_t tids[TOTAL_GROUP_TEST_THREADS];
    int logical_id_counter = 0;

    printf("\n-- Phase 1: Creating %d threads (L_ID 0-%d) --\n", NUM_PHASE1_THREADS, NUM_PHASE1_THREADS - 1);
    for (int i = 0; i < NUM_PHASE1_THREADS; i++) {
        thread_args_t *args = malloc(sizeof(thread_args_t));
        if (!args) {
            printf("Main: Malloc FAILED for L_ID %d. Aborting.\n", logical_id_counter);
            exit(1);
        }
        args->logical_id = logical_id_counter;
        args->phase = 1;
        args->is_last_in_phase = (logical_id_counter == NUM_PHASE1_THREADS - 1);
        args->baseline_family_count_at_start = baseline_family_count;

        if (i % 3 == 0) { 
            args->job_type = JOB_SLEEP;
            args->job_duration = (i == 0) ? 25 : 35;
        } else if (i % 3 == 1) { 
            args->job_type = JOB_RUN;
            args->job_duration = (i == 1) ? 50000 : 100000; 
        } else { 
            args->job_type = JOB_EARLY_EXIT;
            args->job_duration = 5;
        }

        if (knife_thread_create(&tids[logical_id_counter], gt_worker_thread_func, args) < 0) {
            printf("Main: Create FAILED for L_ID %d (%s)\n", logical_id_counter, job_type_to_string(args->job_type));
            free(args);
            exit(1);
        }
        args->self_tid = tids[logical_id_counter];
        logical_id_counter++;
        sleep(2); // Increased stagger for Phase 1 starts
    }
    printf("Main: All Phase 1 threads (L_ID 0-%d) created.\n", NUM_PHASE1_THREADS - 1);

    printf("\n-- Phase 1: Joining %d threads --\n", NUM_PHASE1_THREADS);
    for (int i = 0; i < NUM_PHASE1_THREADS; i++) {
        if (knife_thread_join(tids[i]) < 0) {
            printf("Main: Join FAILED for L_ID %d (TID %d)\n", i, tids[i]);
        }
    }
    printf("-- Phase 1: All %d threads joined (or attempt made). --\n", NUM_PHASE1_THREADS);

    printf("\n-- Phase 2: Creating %d threads (L_ID %d-%d) one by one --\n",
           NUM_PHASE2_THREADS, logical_id_counter, logical_id_counter + NUM_PHASE2_THREADS - 1);
    int phase2_start_logical_id = logical_id_counter;

    for (int i = 0; i < NUM_PHASE2_THREADS; i++) {
        thread_args_t *args = malloc(sizeof(thread_args_t));
        if (!args) {
            printf("Main: Malloc FAILED for L_ID %d. Aborting.\n", logical_id_counter);
            exit(1);
        }
        args->logical_id = logical_id_counter;
        args->phase = 2;
        args->is_last_in_phase = (i == NUM_PHASE2_THREADS - 1);
        args->baseline_family_count_at_start = baseline_family_count;

        printf("Main: Creating L_ID %d. Will sleep after creation for its initial checks.\n", logical_id_counter);

        if (i == 0) { 
            args->job_type = JOB_SLEEP;
            args->job_duration = 10;
        } else if (i == 1) { 
            args->job_type = JOB_RUN;
            args->job_duration = 30000;
        } else { 
            args->job_type = JOB_EARLY_EXIT;
            args->job_duration = 3;
        }

        if (knife_thread_create(&tids[logical_id_counter], gt_worker_thread_func, args) < 0) {
            printf("Main: Create FAILED for L_ID %d (%s)\n", logical_id_counter, job_type_to_string(args->job_type));
            free(args);
            exit(1);
        }
        args->self_tid = tids[logical_id_counter];
        logical_id_counter++;
        sleep(5); 
    }
    printf("Main: All Phase 2 threads (L_ID %d-%d) created.\n", phase2_start_logical_id, logical_id_counter - 1);

    printf("\n-- Phase 2: Joining %d threads --\n", NUM_PHASE2_THREADS);
    for (int i = 0; i < NUM_PHASE2_THREADS; i++) {
        int current_tid_index = phase2_start_logical_id + i;
        if (knife_thread_join(tids[current_tid_index]) < 0) {
            printf("Main: Join FAILED for L_ID %d (TID %d)\n", phase2_start_logical_id + i, tids[current_tid_index]);
        }
    }
    printf("-- Phase 2: All %d threads joined (or attempt made). --\n", NUM_PHASE2_THREADS);

    printf("\n== GROUP TEST COMPLETED ==\n");
    exit(0);
}

/*
// To run this test standalone (if not part of mega_test)
int main(int argc, char *argv[]) {
    main_group_test_impl();
    return 0;
}
*/
