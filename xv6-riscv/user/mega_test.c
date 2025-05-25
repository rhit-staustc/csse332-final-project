#include "kernel/types.h"
#include "user/user.h"

// IMPORTANT ASSUMPTION:
// The following 'main_..._impl' functions are assumed to be the refactored
// main logic from the respective test files. For this mega_test to compile
// and run, you would need to:
// 1. Modify each original test file (e.g., thread_createtest.c):
//    - Rename its main() function (e.g., to main_thread_createtest_impl()).
//    - Ensure it includes any necessary headers it originally did.
//    - Ensure its threads/logic call exit() appropriately for thread_join to work.
// 2. Compile all these modified test files and mega_test.c together,
//    linking them into a single executable named 'mega_test'.

// Forward declarations of the refactored main functions from other test files.
// These functions are expected to encapsulate the entire test logic and
// call exit() when they (or the threads they spawn) are done.
extern void main_thread_createtest_impl(void);
extern void main_combined_test_impl(void);
extern void main_group_test_impl(void);
extern void main_thread_status_test_impl(void);
extern void main_memory_update_test_impl(void);

// Argument structure for the test runner thread
struct test_runner_arg {
    void (*test_func)(void); // Pointer to the test function to run
    const char *test_name;   // Name of the test for logging
};

// Thread function that executes a given test's main logic.
void test_runner_main(void *arg) {
    struct test_runner_arg *test_arg = (struct test_runner_arg *)arg;
    printf("Mega_test: Thread %d starting test: %s\n", getpid(), test_arg->test_name);

    test_arg->test_func(); // Execute the actual test logic.
                           // This function is expected to eventually call exit().

    // Note: If test_func() completes without calling exit(), this thread
    // will exit here. However, the standard pattern for xv6 threads being joined
    // is for the thread function itself (or a function it calls) to call exit().
    printf("Mega_test: Warning - test %s completed without explicit exit in its logic. Thread %d exiting now.\n", test_arg->test_name, getpid());
    exit(0);
}

int main(void) {
    printf("========== Mega Test Suite Starting ==========\n");

    struct test_runner_arg current_test_arg;
    int tid;
    int join_ret;

    // Test 1: thread_createtest.c
    current_test_arg.test_func = main_thread_createtest_impl;
    current_test_arg.test_name = "thread_createtest";
    printf("\n--- Mega_test: Preparing to run %s ---\n", current_test_arg.test_name);
    tid = thread_create(test_runner_main, (void *)&current_test_arg);
    if (tid < 0) {
        printf("Mega_test: FATAL - thread_create failed for %s\n", current_test_arg.test_name);
        exit(1);
    }
    join_ret = thread_join(tid);
    if (join_ret < 0) {
        printf("Mega_test: ERROR - thread_join failed for %s (tid: %d)\n", current_test_arg.test_name, tid);
        // Optionally, exit(1) here or continue with other tests
    } else {
        printf("Mega_test: Successfully joined thread for %s (tid: %d). Test presumed complete.\n", current_test_arg.test_name, tid);
    }
    printf("--- Mega_test: Finished %s ---\n", current_test_arg.test_name);
    sleep(10); // Small delay to allow system to settle if needed, and for observation.

    // Test 2: combined_test.c
    current_test_arg.test_func = main_combined_test_impl;
    current_test_arg.test_name = "combined_test";
    printf("\n--- Mega_test: Preparing to run %s ---\n", current_test_arg.test_name);
    tid = thread_create(test_runner_main, (void *)&current_test_arg);
    if (tid < 0) {
        printf("Mega_test: FATAL - thread_create failed for %s\n", current_test_arg.test_name);
        exit(1);
    }
    join_ret = thread_join(tid);
    if (join_ret < 0) {
        printf("Mega_test: ERROR - thread_join failed for %s (tid: %d)\n", current_test_arg.test_name, tid);
    } else {
        printf("Mega_test: Successfully joined thread for %s (tid: %d). Test presumed complete.\n", current_test_arg.test_name, tid);
    }
    printf("--- Mega_test: Finished %s ---\n", current_test_arg.test_name);
    sleep(10);

    // Test 3: group_test.c
    current_test_arg.test_func = main_group_test_impl;
    current_test_arg.test_name = "group_test";
    printf("\n--- Mega_test: Preparing to run %s ---\n", current_test_arg.test_name);
    tid = thread_create(test_runner_main, (void *)&current_test_arg);
    if (tid < 0) {
        printf("Mega_test: FATAL - thread_create failed for %s\n", current_test_arg.test_name);
        exit(1);
    }
    join_ret = thread_join(tid);
    if (join_ret < 0) {
        printf("Mega_test: ERROR - thread_join failed for %s (tid: %d)\n", current_test_arg.test_name, tid);
    } else {
        printf("Mega_test: Successfully joined thread for %s (tid: %d). Test presumed complete.\n", current_test_arg.test_name, tid);
    }
    printf("--- Mega_test: Finished %s ---\n", current_test_arg.test_name);
    sleep(10);

    // Test 4: thread_status_test.c
    current_test_arg.test_func = main_thread_status_test_impl;
    current_test_arg.test_name = "thread_status_test";
    printf("\n----------- Mega_test: Preparing to run %s -----------\n", current_test_arg.test_name);
    tid = thread_create(test_runner_main, (void *)&current_test_arg);
    if (tid < 0) {
        printf("Mega_test: FATAL - thread_create failed for %s\n", current_test_arg.test_name);
        exit(1);
    }
    join_ret = thread_join(tid);
    if (join_ret < 0) {
        printf("Mega_test: ERROR - thread_join failed for %s (tid: %d)\n", current_test_arg.test_name, tid);
    } else {
        printf("Mega_test: Successfully joined thread for %s (tid: %d). Test presumed complete.\n", current_test_arg.test_name, tid);
    }
    printf("---------------- Mega_test: Finished %s ----------------\n", current_test_arg.test_name);
    sleep(10);

    // Test 5: memory_update_test.c
    current_test_arg.test_func = main_memory_update_test_impl;
    current_test_arg.test_name = "memory_update_test";
    printf("\n--- Mega_test: Preparing to run %s ---\n", current_test_arg.test_name);
    tid = thread_create(test_runner_main, (void *)&current_test_arg);
    if (tid < 0) {
        printf("Mega_test: FATAL - thread_create failed for %s\n", current_test_arg.test_name);
        exit(1);
    }
    join_ret = thread_join(tid);
    if (join_ret < 0) {
        printf("Mega_test: ERROR - thread_join failed for %s (tid: %d)\n", current_test_arg.test_name, tid);
    } else {
        printf("Mega_test: Successfully joined thread for %s (tid: %d). Test presumed complete.\n", current_test_arg.test_name, tid);
    }
    printf("--- Mega_test: Finished %s ---\n", current_test_arg.test_name);

    printf("\n========== Mega Test Suite Finished ==========\n");
    exit(0);
}
