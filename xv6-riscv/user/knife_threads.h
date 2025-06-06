/*
 *
 * User API for using Knife Threads
 *
 */


typedef int knife_thread_t;

// API for thread create
/*
 * Internally, this should call thread_create(), and create a stack for the thread
 * should also keep track of id for the user
 *
 * Takes arguments of thread to create, function for thread to run, and arguments for that function
*/
int knife_thread_create(knife_thread_t *thread, void (*thread_function)(void *), void *arg);


// API for thread join
int knife_thread_join(knife_thread_t thread);


/*
 * Thread Exit Function API
 *
 * Allows a thread to exit early 
 * returns a value, should send a signal to parent process that can be specified, 
 * and should free its resources - clean up stack
 *
*/
// void knife_thread_exit(void *return_val, int signal_num);
void knife_thread_exit(void);

/*
 * Get all thread IDs in the family
 * 
 * Retrieves thread IDs for all threads in the current thread group
 * 
 * @param buf Buffer to store the thread IDs
 * @param max Maximum number of thread IDs to store
 * @return The number of thread IDs stored, or -1 on error
 */
int knife_getfamily(int *buf, int max);

/*
 * Get thread status
 * 
 * Retrieves the status of a thread (active, exited, etc.)
 *
 * @param tid Thread ID to check status for
 * @return Thread status code, or -1 if error/not found
 */
int knife_getstatus(int tid);

