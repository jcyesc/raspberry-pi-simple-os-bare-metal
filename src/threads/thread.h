/*
 * thread.h
 *
 *  Contains the definition of the thread struct and the
 *  thread functions.
 */

#ifndef THREADS_THREAD_H_
#define THREADS_THREAD_H_

#include <debug.h>
#include <stdint.h>
#include "interrupt.h"

#include "../lib/kernel/list.h"

/* States in a thread's life cycle. */
enum thread_status {
  THREAD_RUNNING,       /* Running thread. */
  THREAD_READY,         /* Not running but ready to run. */
  THREAD_BLOCKED,       /* Waiting for an event to trigger. */
  THREAD_DYING          /* About to be destroyed. */
};

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int32_t tid_t;
#define TID_ERROR ((tid_t) -1)          /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                       /* Lowest priority. */
#define PRI_DEFAULT 31                  /* Default priority. */
#define PRI_MAX 63                      /* Highest priority. */

typedef void thread_func(void *parameter);

/* A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion. */

/* The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list. */
struct thread {
  tid_t tid;                      /* Thread identifier */
  enum thread_status status;    /* Thread status. */
  char name[20];                /* Name (for debugging purposes). */
  int32_t priority;             /* Priority */
  thread_func *function;        /* Function to call. */
  void *parameter;              /* Function parameter. */
  struct interrupts_stack_frame stack_frame; /* Stack frame of the thread */

  struct list_elem allelem;     /* List element for all threads list. */
  /* Share between thread.c and synch.c. */
  struct list_elem elem;        /* List element. */

  /* Owned by thread.c. */
  uint32_t magic;               /* Detects stack overflow. */
};

void thread_init(void);
void thread_start();

tid_t thread_create(const char *name, int32_t priority, thread_func *function, void *aux_parameter);

void thread_block(void);
void thread_unblock(struct thread *t);

struct thread *thread_current (void);
tid_t thread_tid (void);
const char *thread_name (void);

void thread_tick (struct interrupts_stack_frame *stack_frame);
void thread_print_stats (void);

void thread_exit (void);
void thread_yield();
void thread_schedule_tail(struct thread *prev, struct thread *next);

/* Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func (struct thread *t, void *aux);
void thread_foreach (thread_action_func *func, void *aux);

int thread_get_priority (void);
void thread_set_priority (int);

int thread_get_nice (void);
void thread_set_nice (int);
int thread_get_recent_cpu (void);
int thread_get_load_avg (void);

#endif /* THREADS_THREAD_H_ */
