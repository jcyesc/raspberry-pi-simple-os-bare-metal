
#include <debug.h>
#include <list.h>
#include <random.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../devices/gpio.h"
#include "../devices/timer.h"
#include "flags.h"
#include "interrupt.h"
#include "palloc.h"
#include "synch.h"
#include "thread.h"
#include "vaddr.h"

/* Returns the value of the current stack pointer. The function is defined
   in interruptsHandlers.s. */
extern void * get_current_sp(void);
/* This function does a context switch by:
    - Saving the stack frame or context of the previous thread.
    - Calling thread_schedule_tail(previous, next).
    - Executing a context switch using the stack frame of the next thread to run.*/
extern void save_and_switch_context(struct interrupts_stack_frame *cur_stack_frame,
    struct interrupts_stack_frame *next_stack_frame, struct thread *cur, struct thread *next);

/* Random value for struct thread's `magic' member.
   Used to detect stack overflow.  See the big comment at the top
   of thread.h for details. */
#define THREAD_MAGIC 0xcd6abf4b

/* List of processes in THREAD_READY state, that is, processes
   that are ready to run but not actually running. */
static struct list ready_list;

/* List of all processes.  Processes are added to this list
   when they are first scheduled and removed when they exit. */
static struct list all_list;

/* Idle thread. */
static struct thread *idle_thread;

/* Initial thread, the thread running init.c:main(). */
static struct thread *initial_thread;

/* Lock used by allocate_tid(). */
static struct lock tid_lock;

/* Statistics. */
static uint64_t idle_ticks;    /* # of timer ticks spent idle. */
static uint64_t kernel_ticks;  /* # of timer ticks in kernel threads. */
static uint64_t user_ticks;    /* # of timer ticks in user programs. */

/* Scheduling. */
#define TIME_SLICE 2            /* # of timer ticks to give each thread. */
static uint32_t thread_ticks;   /* # of timer ticks since last yield. */

/* Stack address to be allocated for the different threads. */
//static uint32_t thread_memory_loc = MEMORY_THREAD_BASE;

static void kernel_thread (thread_func *, void *aux);

/* Current stack frame. */
static struct interrupts_stack_frame *current_stack_frame;
static void set_current_interrupts_stack_frame(struct interrupts_stack_frame *stack_frame);
static struct interrupts_stack_frame *get_current_interrupts_stack_frame();

static void idle (void *idle_started_ UNUSED);
static struct thread* thread_get_running_thread(void);
static struct thread* thread_get_next_thread_to_run(void);
static void thread_save_stack_frame(struct thread* thread, struct interrupts_stack_frame* stack_frame);
static void schedule(); /* Schedule the next thread to run. */
static void schedule_in_interrupt(struct thread *cur, struct thread *next);
static void schedule_not_in_interrupt(struct thread *cur, struct thread *next);
static void context_switch(struct thread* next_thread, struct interrupts_stack_frame* stack_frame);
static bool is_thread (struct thread *t);
static struct thread *get_first_thread();
static tid_t allocate_tid (void);

/* Does basic initialization of t as a blocked thread named NAME. */
static void init_thread (struct thread *t, const char *name, int priority);

/* Initializes the thread system by transforming the code that's currently running into a
   thread. This can't work in general and it is possible in this case only because start.s was
   careful  to put the bottom of the stack at a page boundary. Check start.s for more information.
   (Example: 4080, so pg_round_down() will return 0.).

  Also initializes the run queue and the tid lock.

  After calling this function, be sure to initialize the page allocator before trying
  to create any threads with thread_create().

  It is not safe to call thread_current() until this function finishes.
 */
void thread_init(void) {
  ASSERT(interrupts_get_level() == INTERRUPTS_OFF);

  idle_ticks = 0;
  kernel_ticks = 0;
  user_ticks = 0;

  lock_init (&tid_lock);
  list_init(&ready_list);
  list_init(&all_list);

  /* Set up a thread structure for the running thread. */
  initial_thread = get_first_thread();
  init_thread (initial_thread, "main", PRI_DEFAULT);
  initial_thread->status = THREAD_RUNNING;
  initial_thread->tid = allocate_tid();
}

/* Does basic initialization of T as a blocked thread named NAME.
   Note: This function is only called to initialized the main thread. */
static void init_thread (struct thread *t, const char *name, int priority) {
  ASSERT (t != NULL);
  ASSERT (PRI_MIN <= priority && priority <= PRI_MAX);
  ASSERT (name != NULL);

  memset (t, 0, sizeof *t);
  t->status = THREAD_BLOCKED;

  strlcpy (t->name, name, sizeof t->name);
  /* Sets the stack. It's a full descending stack.*/
  t->stack_frame.r13_sp = get_current_sp();
  t->priority = priority;
  t->magic = THREAD_MAGIC;
  list_push_back (&all_list, &t->allelem);

  /* Setting the current interrupts stack frame. */
  set_current_interrupts_stack_frame(&t->stack_frame);
}

/* Returns a tid to use for a new thread. */
static tid_t allocate_tid (void) {
  static tid_t next_tid = 1;
  tid_t tid;

  lock_acquire (&tid_lock);
  tid = next_tid++;
  lock_release (&tid_lock);

  return tid;
}

/** Gets the first thread that the OS is going to run. */
static struct thread *get_first_thread() {
  void *ptr_sp = get_current_sp();

  /* Copy the current CPU's stack pointer into 'ptr_sp', and then round that
     down to the start of a page. Because 'struct thread' is always at the beginning
     of a page and the stack pointer is somewhere in the middle this locates the
     current thread. */
  return pg_round_down(ptr_sp);
}

/* Starts preemptive thread scheduling by enabling interrupts. */
void thread_start() {
  /* Creating the idle thread. */
  struct semaphore idle_started;
  sema_init (&idle_started, 0);
  thread_create("Idle Thread", PRI_MAX, &idle, &idle_started);

  // Only Enables the IRQ interruptions, FIQ interruptions remain disable.
  interrupts_enable();

  /* Wait for the idle thread to initialize idle_thread. */
  sema_down (&idle_started);
}

/* Called by the timer interrupt handler at each timer tick.
   Thus, this function runs in an external interrupt context. */
void thread_tick (struct interrupts_stack_frame *stack_frame) {
  /* Setting the current stack frame.
     Note: This is the first thing that it has to be done before any further processing. */
  set_current_interrupts_stack_frame(stack_frame);

  struct thread *t = thread_current();

  /* Update statistics. */
  if (t == idle_thread) {
      idle_ticks++;
  } else {
      kernel_ticks++;
  }

  /* Enforce preemption. */
  ++thread_ticks;
  if (thread_ticks >= TIME_SLICE) {
    interrupts_yield_on_return();
  }
}

/* Prints thread statistics. */
void thread_print_stats (void) {
  printf ("Thread: %lld idle ticks, %lld kernel ticks, %lld user ticks\n",
          idle_ticks, kernel_ticks, user_ticks);
}

/* Creates a new kernel thread named NAME with the given initial PRIORITY, which executes
   FUNCTION passing AUX as the argument, and adds it to the ready queue. Returns the thread
   identifier for the new thread, or TID_ERROR if creation fails.

  If thread_start() has been called, then the new thread may be scheduled before thread_create()
  returns. It could even exit before thead_create() returns. Contrariwise, the original thread
  may run for any amount of time before the new thread is scheduled. Use a semaphore or some other
  form of synchronization if you need to ensure ordering.

  The code provided sets the new thread's 'priority' member to PRIORITY, but no actual priority
  scheduling is implemented yet.
  */
tid_t thread_create(const char *name, int32_t priority,
    thread_func *function, void *aux_parameter) {
  ASSERT (PRI_MIN <= priority && priority <= PRI_MAX);
  ASSERT (name != NULL);
  ASSERT (function != NULL);

  enum interrupts_level old_level;
  tid_t tid;

  /* Prepare thread for first run by initializing its stack.
     Do this atomically so intermediate values for the 'stack'
     member cannot be observed. */
  old_level = interrupts_disable ();

  struct thread *thread = palloc_get_page(PAL_ZERO);
  if (thread == NULL) {
      return TID_ERROR;
  }

  // Setting the tid number.
  tid = thread->tid = allocate_tid();

  thread->status = THREAD_BLOCKED;
  strlcpy(thread->name, name, sizeof thread->name);
  thread->priority = priority;
  thread->magic = THREAD_MAGIC;
  thread->function = (thread_func *) function;

  /* Setting the Stack Pointer. Note that we are subtracting -4, so when pg_round_down() is called
     to get the current thread, it returns the right page boundary. */
  thread->stack_frame.r13_sp = (uint32_t *) ((uint8_t *) thread + PGSIZE - 4);

  // Setting the function and the parameters.
  thread->stack_frame.r0 = (uint32_t) function;          // Passing the function as parameter in r0.
  thread->stack_frame.r1 = (uint32_t) aux_parameter;     // Passing the aux_parameter in r1.
  thread->stack_frame.r15_pc = (void *) kernel_thread;

  // Setting the CPSR
  // TODO Change to USER's mode.
  thread->stack_frame.cpsr = SYS_MODE | FLAG_FIQ; // User's mode with the FIQ disable.

  // Setting the return address (Link Register - LR)
  thread->stack_frame.r14_lr = (void *) 0;

  list_push_back(&all_list, &thread->allelem);

  interrupts_set_level(old_level);

  /* Add to run queue. */
  thread_unblock (thread);

  return tid;
}

/* Puts the current thread to sleep.  It will not be scheduled
   again until awoken by thread_unblock().

   This function must be called with interrupts turned off.  It
   is usually a better idea to use one of the synchronization
   primitives in synch.h. */
void thread_block (void) {
  ASSERT (!interrupts_context ());
  ASSERT (interrupts_get_level () == INTERRUPTS_OFF);

  thread_current ()->status = THREAD_BLOCKED;
  schedule ();
}

/* Transitions a blocked thread T to the ready-to-run state.
   This is an error if T is not blocked.  (Use thread_yield() to
   make the running thread ready.)

   This function does not preempt the running thread.  This can
   be important: if the caller had disabled interrupts itself,
   it may expect that it can atomically unblock a thread and
   update other data. */
void thread_unblock (struct thread *t) {
  enum interrupts_level old_level;

  ASSERT (is_thread (t));

  old_level = interrupts_disable ();
  ASSERT (t->status == THREAD_BLOCKED);
  list_push_back (&ready_list, &t->elem);
  t->status = THREAD_READY;
  interrupts_set_level (old_level);
}

/* Returns the name of the running thread. */
const char * thread_name (void) {
  return thread_current()->name;
}

/* Returns the running thread.
   This is thread_get_running_thread() plus a couple of sanity checks.
   See the big comment at the top of thread.h for details. */
struct thread * thread_current (void)  {
  struct thread *t = thread_get_running_thread();

  /* Make sure T is really a thread.
     If either of these assertions fire, then your thread may
     have overflowed its stack.  Each thread has less than 4 kB
     of stack, so a few big automatic arrays or moderate
     recursion can cause stack overflow. */
  ASSERT (is_thread (t));
  ASSERT (t->status == THREAD_RUNNING);

  return t;
}

/* Returns the running thread's tid. */
tid_t thread_tid (void) {
  return thread_current ()->tid;
}

/* Deschedules the current thread and destroys it.  Never
   returns to the caller. */
void thread_exit (void) {
  ASSERT (!interrupts_context ());
  ASSERT (thread_current()->status == THREAD_RUNNING)

  /* Remove thread from all threads list, set our status to dying,
     and schedule another process.  That process will destroy us
     when it calls thread_schedule_tail(). */
  interrupts_disable();
  printf("\nDying slowly ---------------------------------- %s", thread_current()->name);
  list_remove (&thread_current()->allelem);
  thread_current ()->status = THREAD_DYING;
  schedule ();
  NOT_REACHED ();
}

/* Yields the CPU.  The current thread is not put to sleep and
   may be scheduled again immediately at the scheduler's whim. */
void thread_yield() {
  struct thread *cur = thread_current();
  enum interrupts_level old_level;

  ASSERT (!interrupts_context());
  ASSERT(cur->status != THREAD_DYING);

  old_level = interrupts_disable();
  if (cur != idle_thread) {
    list_push_back (&ready_list, &cur->elem);
  }
  cur->status = THREAD_READY;
  schedule();
  interrupts_set_level(old_level);
}

static void schedule() {
  /* Scheduling the next thread to run. */
  struct thread *cur = thread_get_running_thread();
  struct thread *next = thread_get_next_thread_to_run();

  ASSERT (interrupts_get_level () == INTERRUPTS_OFF);
  ASSERT (cur->status != THREAD_RUNNING);
  ASSERT (is_thread (next));

  printf("\nKernel Scheduler");

  if (interrupts_was_irq_generated()) {
    printf("\nScheduling a thread in interrupt.");
    schedule_in_interrupt(cur, next);
  } else {
    printf("\nScheduling a thread not in interrupt.");
    schedule_not_in_interrupt(cur, next);
  }
}

/* When an IRQ interrupts is generated, the interrupt_stack_frame is saved and by the interrupts
   framework and we can work directly with using the method get_current_interrupts_stack_frame(). */
static void schedule_in_interrupt(struct thread *cur, struct thread *next) {
  ASSERT (interrupts_get_level () == INTERRUPTS_OFF);
  ASSERT (is_thread (cur));
  ASSERT (is_thread (next));
  ASSERT(interrupts_was_irq_generated());

  /* Save the current stack frame of the current thread. */
  thread_save_stack_frame(cur, get_current_interrupts_stack_frame());

  if (cur != next) {
      /* Makes a context switch and sets the values for the new stack. */
      context_switch(next, get_current_interrupts_stack_frame());
  }
  thread_schedule_tail(cur, next);
}

/* When the schedule method was called by a thread, and an interrupt was not generated, we
   need to save the context of that thread and do the context switch in assembly because
   when the thread that called the scheduler is scheduled again, it is going to start execution
   in this function. This case happens when the methods thread_exit(), thread_block() are called. */
static void schedule_not_in_interrupt(struct thread *cur, struct thread *next) {
  ASSERT (interrupts_get_level () == INTERRUPTS_OFF);
  ASSERT (is_thread (cur));
  ASSERT (is_thread (next));
  ASSERT(!interrupts_was_irq_generated());

  save_and_switch_context(&cur->stack_frame, &next->stack_frame, cur, next);
}

/* Completes a thread switch by activating the new thread's page tables, and if the previous
   thread is dying, destroying it.

   This function is normally invoked by schedule() as its final action before returning.

   After this function and its caller returns, the thread switch is complete.
 */
void thread_schedule_tail(struct thread *prev, struct thread *next) {
  ASSERT (interrupts_get_level () == INTERRUPTS_OFF);

  printf("\nSchedule tail");
  printf("\nPrev: %s, TID: %d", prev->name, prev->tid);
  printf("\nNext: %s, TID: %d", next->name, next->tid);

  /* Start new time slice. */
  thread_ticks = 0;

  /* Mark us as running. */
  next->status = THREAD_RUNNING;

  /* If the thread we switched from is dying, destroy its struct
     thread.  This must happen late so that thread_exit() doesn't
     pull out the rug under itself.  (We don't free
     initial_thread because its memory was not obtained via
     palloc().) */
  if (prev->status == THREAD_DYING && prev != initial_thread) {
       ASSERT (prev != next)
       printf("\nReleasing resources of : %s, TID: %d", prev->name, prev->tid);

       /* Releasing the memory that was assigned to this thread. */
       palloc_free_page(prev);
       timer_msleep(1000000);
   }
}

/* Invoke function 'func' on all threads, passing along 'aux'.
   This function must be called with interrupts off. */
void thread_foreach (thread_action_func *func, void *aux) {
  struct list_elem *e;

  ASSERT (interrupts_get_level () == INTERRUPTS_OFF);

  for (e = list_begin (&all_list); e != list_end (&all_list);
      e = list_next (e)) {
      struct thread *t = list_entry (e, struct thread, allelem);
      func (t, aux);
    }
}

/* Sets the current thread's priority to NEW_PRIORITY. */
void thread_set_priority (int new_priority) {
  thread_current ()->priority = new_priority;
}

/* Returns the current thread's priority. */
int thread_get_priority (void) {
  return thread_current ()->priority;
}

/* Sets the current thread's nice value to NICE. */
void thread_set_nice (int nice UNUSED) {
  /* Not yet implemented. */
}

/* Returns the current thread's nice value. */
int thread_get_nice (void) {
  /* Not yet implemented. */
  return 0;
}

/* Returns 100 times the system load average. */
int thread_get_load_avg (void) {
  /* Not yet implemented. */
  return 0;
}

/* Returns 100 times the current thread's recent_cpu value. */
int thread_get_recent_cpu (void) {
  /* Not yet implemented. */
  return 0;
}


/* Idle thread. Executes when no other thread is ready to run.

  The idle thread is initially put on the ready list by thread_start(). It will be scheduled
  once initially, at which point it initializes idle_thread, "up"s the semaphore passed to it
  to enable thread_start() to continue, and immediately blocks. After that, the idle thread never
  appears in the ready list. It is returned by thread_get_next_thread_to_run() as a special
  case when the ready list is empty. */
static void idle (void *idle_started_ UNUSED) {
  ASSERT(idle_started_ != NULL);

  struct semaphore *idle_started = idle_started_;
  idle_thread = thread_current();
  sema_up(idle_started);

  unsigned short green = 0x7E0;

  for(;;) {
      SetForeColour(green);
      printf("\nIdle thread....");
      timer_msleep(1000000);

      /* Let someone else run. */
      enum interrupts_level old_level = interrupts_disable();
      thread_block();
      interrupts_set_level(old_level);

  }
}

/* Function used as the basis for a kernel thread. */
static void kernel_thread (thread_func *function, void *aux) {
  ASSERT (function != NULL);

  interrupts_enable();  /* The scheduler runs with interrupts off. */
  function(aux);        /* Execute the thread function. */
  thread_exit();        /* If function() returns, kill the thread. */
}

static struct thread* thread_get_running_thread(void) {
  uint32_t *sp;
  if (interrupts_was_irq_generated()) {
      // Get the stack pointer frame the interrupts stack frame that was save.
      struct interrupts_stack_frame *stack_frame = get_current_interrupts_stack_frame();
      sp = stack_frame->r13_sp;
  } else {
      // No interrupt was generated, so get the current stack pointer. (The stack pointer of the
      // user's thread, not the banked IRQ stack pointer.
      sp = get_current_sp();
  }

  /* Copy the current CPU's stack pointer into 'ptr_sp', and then round that
     down to the start of a page. Because 'struct thread' is always at the beginning
     of a page and the stack pointer is somewhere in the middle this locates the
     current thread. */
  return pg_round_down((void *) sp);
}

/* Returns true if T appears to point to a valid thread. */
static bool is_thread (struct thread *t) {
  return t != NULL && t->magic == THREAD_MAGIC;
}

/* Chooses and returns the next thread to be scheduled. Should return a thread from the run queue,
 * unless the run queue is empty. (If the running thread can continue running, then it will be in
 * the run queue.) If the run queue is empty, return idle_thred.
 */
static struct thread* thread_get_next_thread_to_run(void) {
  if (list_empty(&ready_list)) {
	  // thread_unblock(idle_thread);
      return idle_thread;
  } else {
      return list_entry (list_pop_front (&ready_list), struct thread, elem);
  }
}

static void thread_save_stack_frame(struct thread* thread, struct interrupts_stack_frame* stack_frame) {
  /* Note: If the string.h library is not supported, the next assignment can't be done because the
   * GNU GCC compiler uses memcpy() function to copy the data. */
  thread->stack_frame = *stack_frame;
  /*thread->stack_frame.cpsr = stack_frame->cpsr;
  thread->stack_frame.r0 = stack_frame->r0;
  thread->stack_frame.r1 = stack_frame->r1;
  thread->stack_frame.r2 = stack_frame->r2;
  thread->stack_frame.r3 = stack_frame->r3;
  thread->stack_frame.r4 = stack_frame->r4;
  thread->stack_frame.r5 = stack_frame->r5;
  thread->stack_frame.r6 = stack_frame->r6;
  thread->stack_frame.r7 = stack_frame->r7;
  thread->stack_frame.r8 = stack_frame->r8;
  thread->stack_frame.r9 = stack_frame->r9;
  thread->stack_frame.r10 = stack_frame->r10;
  thread->stack_frame.r11 = stack_frame->r11;
  thread->stack_frame.r12 = stack_frame->r12;
  thread->stack_frame.r13_sp = stack_frame->r13_sp;
  thread->stack_frame.r14_lr = stack_frame->r14_lr;
  thread->stack_frame.r15_pc = stack_frame->r15_pc;
  */
}

static void context_switch(struct thread* next, struct interrupts_stack_frame* stack_frame) {
  /* Note: If the string.h library is not supported, the next assignment can't be done because the
   * GNU GCC compiler uses memcpy() function to copy the data. */
  *stack_frame = next->stack_frame;
  /*
  stack_frame->cpsr = next->stack_frame.cpsr;
  stack_frame->r0 = next->stack_frame.r0;
  stack_frame->r1 = next->stack_frame.r1;
  stack_frame->r2 = next->stack_frame.r2;
  stack_frame->r3 = next->stack_frame.r3;
  stack_frame->r4 = next->stack_frame.r4;
  stack_frame->r5 = next->stack_frame.r5;
  stack_frame->r6 = next->stack_frame.r6;
  stack_frame->r7 = next->stack_frame.r7;
  stack_frame->r8 = next->stack_frame.r8;
  stack_frame->r9 = next->stack_frame.r9;
  stack_frame->r10 = next->stack_frame.r10;
  stack_frame->r11 = next->stack_frame.r11;
  stack_frame->r12 = next->stack_frame.r12;
  stack_frame->r13_sp = next->stack_frame.r13_sp;
  stack_frame->r14_lr = next->stack_frame.r14_lr;
  stack_frame->r15_pc = next->stack_frame.r15_pc;
  */
}

/* Sets the current interrupts stack frame. */
static void set_current_interrupts_stack_frame(struct interrupts_stack_frame *stack_frame) {
  ASSERT(stack_frame != NULL)
  current_stack_frame = stack_frame;
}

/* Returns the current interrupts stack frame. */
static struct interrupts_stack_frame *get_current_interrupts_stack_frame() {
  ASSERT(current_stack_frame != NULL)
  return current_stack_frame;
}
