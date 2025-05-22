#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include <stddef.h>

struct cpu cpus[NCPU];

struct proc proc[NPROC];

struct proc *initproc;

int nextpid = 1;
struct spinlock pid_lock;
struct spinlock proc_table_lock;

extern void forkret(void);
static void freeproc(struct proc *p);

extern char trampoline[]; // trampoline.S

// helps ensure that wakeups of wait()ing
// parents are not lost. helps obey the
// memory model when using p->parent.
// must be acquired before any p->lock.
struct spinlock wait_lock;

// Allocate a page for each process's kernel stack.
// Map it high in memory, followed by an invalid
// guard page.
void
proc_mapstacks(pagetable_t kpgtbl)
{
  struct proc *p;
  
  for(p = proc; p < &proc[NPROC]; p++) {
    char *pa = kalloc();
    if(pa == 0)
      panic("kalloc");
    uint64 va = KSTACK((int) (p - proc));
    kvmmap(kpgtbl, va, (uint64)pa, PGSIZE, PTE_R | PTE_W);
  }
}

// initialize the proc table.
void
procinit(void)
{
  struct proc *p;
  initlock(&pid_lock, "nextpid");
  initlock(&wait_lock, "wait_lock");
  initlock(&proc_table_lock, "proc_table");
  for(p = proc; p < &proc[NPROC]; p++) {
      initlock(&p->lock, "proc");
      p->state = UNUSED;
      p->kstack = KSTACK((int) (p - proc));
  }
}

// Must be called with interrupts disabled,
// to prevent race with process being moved
// to a different CPU.
int
cpuid()
{
  int id = r_tp();
  return id;
}

// Return this CPU's cpu struct.
// Interrupts must be disabled.
struct cpu*
mycpu(void)
{
  int id = cpuid();
  struct cpu *c = &cpus[id];
  return c;
}

// Return the current struct proc *, or zero if none.
struct proc*
myproc(void) 
{
  struct cpu *c = mycpu();
  return c->proc;
}

struct proc*
myproc_safe(void) 
{
  push_off();
  struct cpu *c = mycpu();
  struct proc *p = c->proc;
  pop_off();
  return p;
}

int
allocpid()
{
  int pid;
  
  acquire(&pid_lock);
  pid = nextpid;
  nextpid = nextpid + 1;
  release(&pid_lock);

  return pid;
}

// Look in the process table for an UNUSED proc.
// If found, initialize state required to run in the kernel,
// and return with p->lock held.
// If there are no free procs, or a memory allocation fails, return 0.
static struct proc*
allocproc(int alloc_pagetable)
{
  struct proc *p;

  for(p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if(p->state == UNUSED) {
      goto found;
    } else {
      release(&p->lock);
    }
  }
  return 0;

found:
  p->pid = allocpid();
  p->state = USED;

  p->group_leader = NULL;
  p->group_prev = NULL;
  p->group_next = NULL;
  p->num_children = 0;
  p->ref_count = 0;

  // Allocate a trapframe page.
  if((p->trapframe = (struct trapframe *)kalloc()) == 0){
    freeproc(p);
    release(&p->lock);
    return 0;
  }
  // An empty user page table.
  p->pagetable = proc_pagetable(p);
  if(p->pagetable == 0){
    freeproc(p);
    release(&p->lock);
    return 0;
  }
  
  // Set up new context to start executing at forkret,
  // which returns to user space.
  memset(&p->context, 0, sizeof(p->context));
  p->context.ra = (uint64)forkret;
  p->context.sp = p->kstack + PGSIZE;

  return p;
}

// free a proc structure and the data hanging from it,
// including user pages
// p->lock must be held.
static void
freeproc(struct proc *p)
{
  //if its a user-level thread, only free its trapframe, leave pagetable alone
  if(p->is_thread) {
      if(p->group_leader->ref_count > 0) {
        p->group_leader->ref_count--;
      }

      if(p == p->group_leader && p->group_leader->ref_count != 0) {
        panic("freeproc: leader with ref_count > 0");
      }

      if(p != p->group_leader) {
        //printf("freeproc: non-leader thread\n");
        return;
      }
      
      // If ref_count is 0, free all other threads in the group
        struct proc *q = p->group_next;
        struct proc *next;
        
        // Free all non-leader threads in the group
        while(q != p) {
          next = q->group_next;  // Save next before modifying q
          
          acquire(&q->lock);
          // Only free trapframe for non-leader threads
          if(q->trapframe) {
            kfree((void*)q->trapframe);
            q->trapframe = 0;
          }

          if(q->pagetable) {
            proc_freepagetable_thread(q->pagetable, q->sz);
            q->pagetable = 0;
            q->sz = 0;
          }

          q->group_leader = 0;
          q->group_next = 0;
          q->group_prev = 0;
          q->is_thread = 0;
          q->tid = 0;
          q->cwd = 0;

          q->state = UNUSED;
          release(&q->lock);
          
          q = next;
        }
      
      
      // Now free the leader's resources (self)
      if(p->trapframe) {
        kfree((void*)p->trapframe);
        p->trapframe = 0;
      }
      
      if(p->pagetable) {
        proc_freepagetable(p->pagetable, p->sz); // Only the leader should free the physical page 
        p->pagetable = 0;
        p->sz = 0;
        p->group_leader = 0;
        p->group_next = 0;
        p->group_prev = 0;
        p->is_thread = 0;
        p->tid = 0;
      }
      
      p->state = UNUSED;
      return;
    } else {
      //if full process, full tear-down
        if(p->trapframe) {
          kfree((void*)p->trapframe);
          p->trapframe = 0;
        }
        if(p->pagetable) {
          proc_freepagetable(p->pagetable, p->sz);
          p->pagetable = 0;
          p->sz = 0;
        }
        
        // Always clean up these fields, even if pagetable is NULL
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->chan = 0;
        p->killed = 0;
        p->xstate = 0;
        p->group_leader = 0;
        p->group_next = 0;
        p->group_prev = 0;
        p->is_thread = 0;
        p->tid = 0;
        p->state = UNUSED;
    }
  
}

// Create a user page table for a given process, with no user memory,
// but with trampoline and trapframe pages.
pagetable_t
proc_pagetable(struct proc *p)
{
  pagetable_t pagetable;

  // An empty page table.
  pagetable = uvmcreate();
  if(pagetable == 0)
    return 0;

  // map the trampoline code (for system call return)
  // at the highest user virtual address.
  // only the supervisor uses it, on the way
  // to/from user space, so not PTE_U.
  if(mappages(pagetable, TRAMPOLINE, PGSIZE,
              (uint64)trampoline, PTE_R | PTE_X) < 0){
    uvmfree(pagetable, 0);
    return 0;
  }

  // map the trapframe page just below the trampoline page, for
  // trampoline.S.
  if(mappages(pagetable, TRAPFRAME, PGSIZE,
              (uint64)(p->trapframe), PTE_R | PTE_W) < 0){
    uvmunmap(pagetable, TRAMPOLINE, 1, 0);
    uvmfree(pagetable, 0);
    return 0;
  }

  return pagetable;
}

// Free a process's page table, and free the
// physical memory it refers to.
void
proc_freepagetable(pagetable_t pagetable, uint64 sz)
{
  uvmunmap(pagetable, TRAMPOLINE, 1, 0);
  uvmunmap(pagetable, TRAPFRAME, 1, 0);
  uvmfree_shared(pagetable, sz); // use refcount aware free
}

//this version does NOT free the physical page at the end to avoid double free
void
proc_freepagetable_thread(pagetable_t pagetable, uint64 sz) {
  uvmunmap(pagetable, TRAMPOLINE, 1, 0);
  uvmunmap(pagetable, TRAPFRAME, 1, 0);
  uvmfree_shared_thread(pagetable, sz); // use refcount aware free
}

// a user program that calls exec("/init")
// assembled from ../user/initcode.S
// od -t xC ../user/initcode
// od -t xC ../user/initcode
uchar initcode[] = {
  0x17, 0x05, 0x00, 0x00, 0x13, 0x05, 0x45, 0x02,
  0x97, 0x05, 0x00, 0x00, 0x93, 0x85, 0x35, 0x02,
  0x93, 0x08, 0x70, 0x00, 0x73, 0x00, 0x00, 0x00,
  0x93, 0x08, 0x20, 0x00, 0x73, 0x00, 0x00, 0x00,
  0xef, 0xf0, 0x9f, 0xff, 0x2f, 0x69, 0x6e, 0x69,
  0x74, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};

// Set up first user process.
void
userinit(void)
{
  struct proc *p;

  p = allocproc(1);
  initproc = p;
  
  // allocate one user page and copy initcode's instructions
  // and data into it.
  uvmfirst(p->pagetable, initcode, sizeof(initcode));
  p->sz = PGSIZE;

  // prepare for the very first "return" from kernel to user.
  p->trapframe->epc = 0;      // user program counter
  p->trapframe->sp = PGSIZE;  // user stack pointer

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  p->state = RUNNABLE;

  release(&p->lock);
}

// Grow or shrink user memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint64 sz;
  struct proc *p = myproc();

  sz = p->sz;
  if(n > 0){
    if((sz = uvmalloc(p->pagetable, sz, sz + n, PTE_W)) == 0) {
      return -1;
    }
  } else if(n < 0){
    sz = uvmdealloc(p->pagetable, sz, sz + n);
  }
  p->sz = sz;
  return 0;
}

// Create a new process, copying the parent.
// Sets up child kernel stack to return as if from fork() system call.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *p = myproc();

  // Allocate process.
  if((np = allocproc(1)) == 0){
    return -1;
  }

  // Copy user memory from parent to child.
  if(uvmcopy(p->pagetable, np->pagetable, p->sz) < 0){
    printf("fork: uvmcopy failed\n");
    freeproc(np);
    release(&np->lock);
    return -1;
  }
  np->sz = p->sz;

  // copy saved user registers.
  *(np->trapframe) = *(p->trapframe);

  // Cause fork to return 0 in the child.
  np->trapframe->a0 = 0;

  // increment reference counts on open file descriptors.
  for(i = 0; i < NOFILE; i++)
    if(p->ofile[i])
      np->ofile[i] = filedup(p->ofile[i]);
  np->cwd = idup(p->cwd);

  safestrcpy(np->name, p->name, sizeof(p->name));

  pid = np->pid;

  release(&np->lock);

  acquire(&wait_lock);
  np->parent = p;
  release(&wait_lock);

  acquire(&np->lock);
  np->state = RUNNABLE;
  release(&np->lock);

  return pid;
}

// Pass p's abandoned children to init.
// Caller must hold wait_lock.
void
reparent(struct proc *p)
{
  struct proc *pp;

  for(pp = proc; pp < &proc[NPROC]; pp++){
    if(pp->parent == p){
      pp->parent = initproc;
      wakeup(initproc);
    }
  }
}

// Thread exit function that cleans up thread resources
void 
thread_exit(void)
{
    struct proc *p = myproc();
    
    if(p == 0) {
        panic("thread_exit: no process");
    }
    
    acquire(&p->lock);
    
    // Check if this is actually a thread
    if (!p->is_thread) {
        printf("thread_exit called by non-thread process\n");
        release(&p->lock);
        exit(-1);
        return;
    }
    
    // Get the thread group leader
    struct proc *leader = p->group_leader ? p->group_leader : p;
    
    // Update thread group linked list
    if (p->group_next != p) {
        // Get the leader's lock if this is not the leader
        if (p != leader) {
            acquire(&leader->lock);
        }
        
        // Remove this thread from the circular list
        p->group_prev->group_next = p->group_next;
        p->group_next->group_prev = p->group_prev;
        
        // Update reference count
        if (leader->ref_count > 0) {
            leader->ref_count--;
        }
        
        // If this is the leader but not the last thread, promote a new leader
        if (p == leader && leader->ref_count > 0 && p->group_next) {
            struct proc *new_leader = p->group_next;
            acquire(&new_leader->lock);
            
            // Update all threads to point to the new leader
            struct proc *t = new_leader;
            do {
                if (t != new_leader) {
                    acquire(&t->lock);
                }
                t->group_leader = new_leader;
                if (t != new_leader) {
                    release(&t->lock);
                }
                t = t->group_next;
            } while (t != new_leader);
            
            release(&new_leader->lock);
        }
        
        // Release leader's lock if acquired
        if (p != leader) {
            release(&leader->lock);
        }
    }
    
    // Mark as zombie and set exit status
    p->xstate = 0;  // Exit status 0 for normal thread exit
    p->state = ZOMBIE;
    
    // Wake up any thread that might be waiting to join this one
    acquire(&wait_lock);
    if (p->parent) {
        wakeup(p->parent);
    }
    release(&wait_lock);
    
      
    // Jump into the scheduler, never to return
    sched();
    panic("zombie thread exit");
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait().
void 
exit(int status)
{
    struct proc *p = myproc();
    
    if (p->is_thread) {
        // For threads, use thread_exit
        printf("thread exit\n");
        thread_exit();
        return; // Never reached
    }
    
    acquire(&p->lock);
    
    // This is a process, clean up all resources
    
    // Free open files
    for (int fd = 0; fd < NOFILE; fd++) {
        if (p->ofile[fd]) {
            fileclose(p->ofile[fd]);
            p->ofile[fd] = 0;
        }
    }
    
    // Free current working directory
    begin_op();
    if (p->cwd) {
        iput(p->cwd);
    }
    end_op();
    p->cwd = 0;
    
    // Reparent children to init
    acquire(&wait_lock);
    reparent(p);
    release(&wait_lock);
    
    // Free page table and memory if not already done
    if (p->pagetable) {
        proc_freepagetable(p->pagetable, p->sz);
        p->pagetable = 0;
        p->sz = 0;
    }
    
    // Set state to ZOMBIE and notify parent
    p->xstate = status;
    p->state = ZOMBIE;
    
    // Wake up parent
    acquire(&wait_lock);
    if (p->parent) {
        wakeup(p->parent);
    }
    release(&wait_lock);
    
    // Schedule
    sched();
    panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(uint64 addr)
{
  struct proc *pp;
  int havekids, pid;
  struct proc *p = myproc();

  acquire(&wait_lock);

  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(pp = proc; pp < &proc[NPROC]; pp++){
      if(pp->parent == p){
        // make sure the child isn't still in exit() or swtch().
        acquire(&pp->lock);

        havekids = 1;
        if(pp->state == ZOMBIE){
          // Found one.
          pid = pp->pid;
          if(addr != 0 && copyout(p->pagetable, addr, (char *)&pp->xstate,
                                  sizeof(pp->xstate)) < 0) {
            release(&pp->lock);
            release(&wait_lock);
            return -1;
          }
          //printf("wait: freeproc\n");
          freeproc(pp);
          release(&pp->lock);
          release(&wait_lock);
          return pid;
        }
        release(&pp->lock);
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || killed(p)){
      release(&wait_lock);
      return -1;
    }
    
    // Wait for a child to exit.
    sleep(p, &wait_lock);  //DOC: wait-sleep
  }
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run.
//  - swtch to start running that process.
//  - eventually that process transfers control
//    via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  
  c->proc = 0;
  for(;;){
    // Avoid deadlock by ensuring that devices can interrupt.
    intr_on();

    for(p = proc; p < &proc[NPROC]; p++) {
      acquire(&p->lock);
      if(p->state == RUNNABLE) {
        // Switch to chosen process.  It is the process's job
        // to release its lock and then reacquire it
        // before jumping back to us.
        p->state = RUNNING;
        c->proc = p;
        swtch(&c->context, &p->context);

        // Process is done running for now.
        // It should have changed its p->state before coming back.
        c->proc = 0;
      }
      release(&p->lock);
    }
  }
}

// Switch to scheduler.  Must hold only p->lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->noff, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&p->lock)) {
    printf("Lock: %p\n", &p->lock);
    panic("sched p->lock");
  }
  if(mycpu()->noff != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(intr_get())
    panic("sched interruptible");

  intena = mycpu()->intena;
  swtch(&p->context, &mycpu()->context);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  struct proc *p = myproc();
  acquire(&p->lock);
  p->state = RUNNABLE;
  sched();
  release(&p->lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch to forkret.
void
forkret(void)
{
  static int first = 1;
  struct proc *p = myproc();
  
  // Still holding p->lock from scheduler.
  release(&p->lock);  // CRITICAL: Must release the lock acquired by scheduler

  // Clear first to avoid a race with the first process
  // being created.
  if (first) {
    first = 0;
    fsinit(ROOTDEV);
  }

  // Check if this is a thread
  if (p->is_thread) {
    // For threads, don't run initcode - go directly to usertrapret
    usertrapret();
  }

  usertrapret();
}

// Thread's first return to user space
void
threadret(void)
{
  struct proc *p = myproc();
  
  // Still holding p->lock from scheduler.
  release(&p->lock);
  
  // Make sure we're properly set up
  p->trapframe->epc = (uint64)p->trapframe->epc;
  p->trapframe->sp = PGROUNDDOWN(p->trapframe->sp);
  
  usertrapret();
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  // Must acquire p->lock in order to
  // change p->state and then call sched.
  // Once we hold p->lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup locks p->lock),
  // so it's okay to release lk.

  acquire(&p->lock);  //DOC: sleeplock1
  release(lk);

  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  release(&p->lock);
  acquire(lk);
}

// Wake up all processes sleeping on chan.
// Must be called without any p->lock.
void
wakeup(void *chan)
{
  struct proc *p;

  for(p = proc; p < &proc[NPROC]; p++) {
    if(p != myproc()){
      acquire(&p->lock);
      if(p->state == SLEEPING && p->chan == chan) {
        p->state = RUNNABLE;
      }
      release(&p->lock);
    }
  }
}

// Kill the process with the given pid.
// The victim won't exit until it tries to return
// to user space (see usertrap() in trap.c).
int
kill(int pid)
{
  struct proc *p;

  for(p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);
    if(p->pid == pid){
      p->killed = 1;
      if(p->state == SLEEPING){
        // Wake process from sleep().
        p->state = RUNNABLE;
      }


      //maybe?
      // if(p->state == RUNNABLE) {
      //   // Switch to chosen process
      //   p->state = RUNNING;
      //   c->proc = p;
      //   swtch(&c->context, &p->context);
      //   c->proc = 0;
    //}

      release(&p->lock);
      return 0;
    }
    release(&p->lock);
  }
  return -1;
}

void
setkilled(struct proc *p)
{
  acquire(&p->lock);
  p->killed = 1;
  release(&p->lock);
}

int
killed(struct proc *p)
{
  int k;
  
  acquire(&p->lock);
  k = p->killed;
  release(&p->lock);
  return k;
}

// Copy to either a user address, or kernel address,
// depending on usr_dst.
// Returns 0 on success, -1 on error.
int
either_copyout(int user_dst, uint64 dst, void *src, uint64 len)
{
  struct proc *p = myproc();
  if(user_dst){
    return copyout(p->pagetable, dst, src, len);
  } else {
    memmove((char *)dst, src, len);
    return 0;
  }
}

// Copy from either a user address, or kernel address,
// depending on usr_src.
// Returns 0 on success, -1 on error.
int
either_copyin(void *dst, int user_src, uint64 src, uint64 len)
{
  struct proc *p = myproc();
  if(user_src){
    return copyin(p->pagetable, dst, src, len);
  } else {
    memmove(dst, (char*)src, len);
    return 0;
  }
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [USED]      "used",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  struct proc *p;
  char *state;

  printf("\n");
  for(p = proc; p < &proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    printf("%d %s %s", p->pid, state, p->name);
    printf("\n");
  }
}

uint64 spoon(void *arg)
{
	printf("In spoon system call with argument %p\n", arg);
	return 0;
}

uint64 sys_getfamily(void) {
	int *buf, max;
	struct proc *cur = myproc();

	argaddr(0, (uint64*)&buf);
	argint(1, &max);

	struct proc *leader = cur->group_leader ? cur->group_leader : cur;

	int count = 0;
	struct proc *p = leader;
	do {
		if(count >= max)
			break;
		
		if(copyout(cur->pagetable,
			(uint64)buf + count * sizeof(int),
			(char*)&p->tid,
			sizeof(int)) < 0) {

			return -1;
		}

		count++;
		p = p->group_next;
	} while(p != leader);

	return count;
}


uint64 thread_create(void (*start_routine)(void*), void *arg)
{
    struct proc *p = myproc();
    struct proc *np;
    uint64 curr_group_sz, new_group_sz;
    void *stack_page_physical;
    struct proc *leader;

    // Allocate process with allocproc(0) to skip page table allocation
    if((np = allocproc(0)) == 0) {
        return -1;
    }

    // Get the group leader while holding p's lock
    acquire(&p->lock);
    leader = p->group_leader ? p->group_leader : p;
    
    // Release p's lock before acquiring leader's lock to avoid deadlock
    if (leader != p) {
        release(&p->lock);
        acquire(&leader->lock);
        // Now re-acquire p's lock if needed
        if (p != leader) {
            acquire(&p->lock);
        }
    }

    // Initialize thread's group information
    if (p->group_leader == NULL) {
        // First thread in the group
        p->group_leader = p;
        p->group_next = p;
        p->group_prev = p;
        p->num_children = 0;
        p->ref_count = 1;
    }

    // Update group information
    leader->num_children++;
    leader->ref_count++;
    np->tid = leader->num_children;
    np->group_leader = leader;
    
    // Insert into circular list
    np->group_prev = leader;
    np->group_next = leader->group_next;
    leader->group_next->group_prev = np;
    leader->group_next = np;

    // Share the page table
    curr_group_sz = p->sz;
    uvmshare(p->pagetable, np->pagetable, p->sz);
    new_group_sz = curr_group_sz + PGSIZE;
    

    // Allocate stack page
    if((stack_page_physical = kalloc()) == 0) {
        goto bad;
    }
    memset(stack_page_physical, 0, PGSIZE);

     // Update size for all threads in group
     // map the stack page
    struct proc *q = leader;
    do {
        q->sz = new_group_sz;

        mappages(q->pagetable, curr_group_sz, PGSIZE, 
      (uint64)stack_page_physical, PTE_W|PTE_R|PTE_U);

        q = q->group_next;
    } while (q != leader);

    // First copy the parent's trapframe
    *np->trapframe = *p->trapframe;    
    np->trapframe->epc = (uint64)start_routine;  // Thread entry point
    np->trapframe->sp = curr_group_sz + PGSIZE - 8;
    np->trapframe->a0 = (uint64)arg; //arg passing
    np->trapframe->ra = 0;  // Will cause trap if function returns without explicit exit

    // Copy file descriptors
    for(int i = 0; i < NOFILE; i++) {
        if(p->ofile[i]) {
            np->ofile[i] = filedup(p->ofile[i]);
        }
    }
    np->cwd = idup(p->cwd);

    // Set thread properties
    np->is_thread = 1;
    safestrcpy(np->name, "kthread", sizeof("kthread"));
    np->parent = p;
    np->state = RUNNABLE;

    // Release locks
    release(&p->lock);
    if (leader != p) {
        release(&leader->lock);
    }
    release(&np->lock);

    return np->tid;

bad:
    // Cleanup on error
    printf("Thread creation failed\n");
    np->group_prev->group_next = np->group_next;
    np->group_next->group_prev = np->group_prev;
    leader->num_children--;
    leader->ref_count--;

    release(&p->lock);
    if (leader != p && leader) {
        release(&leader->lock);
    }
    release(&np->lock);
    return -1;
}

uint64 thread_join(int thread_id)
{
    struct proc *p = myproc_safe();
    struct proc *tp;
    int havekids;
    
    acquire(&wait_lock);  // For wait()
    
    for(;;) {
        // Scan through table looking for exited children
        havekids = 0;
        for(tp = proc; tp < &proc[NPROC]; tp++) {
            if(tp->parent == p && tp->tid == thread_id && tp->is_thread) {
                acquire(&tp->lock);
                havekids = 1;
                if(tp->state == ZOMBIE) {
                    // Found one
                    int tid = tp->tid;  // Store tid before freeing
                    
                    //printf("thread_join: freeproc\n");
                    freeproc(tp);
                    release(&tp->lock);
                    release(&wait_lock);
                    return tid;
                }
                release(&tp->lock);
            }
        }
        
        if(!havekids || p->killed) {
            release(&wait_lock);
            return -1;
        }
        
        // Wait for a child to exit
        sleep(p, &wait_lock);
    }
}
