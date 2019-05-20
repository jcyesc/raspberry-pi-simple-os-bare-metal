
#include <debug.h>
#include <random.h>
#include <stdbool.h>
#include "stdio.h"
#include <stdint.h>
#include <string.h>

#include "../devices/gpio.h"
#include "../devices/framebuffer.h"
#include "../devices/serial.h"
#include "../devices/timer.h"
#include "../devices/video.h"
#include "interrupt.h"
#include "init.h"
#include "palloc.h"
#include "malloc.h"
#include "synch.h"
#include "thread.h"
#include "vaddr.h"

/* -ul: Maximum number of pages to put into palloc's user pool. */
static size_t user_page_limit = SIZE_MAX;

/* Tasks for the Threads. */
static void task_0(void *);
static void task_1(void *);
static void task_2(void *);
static void task_3(void *);
static void task_4(void *);
static void task_5(void *);
static void task_6(void *);
static void init_all_threads();
static struct lock lock_task;

/*
static void test_swi_interrupt() {
  unsigned short blue = 0x1f;
  unsigned short green = 0x7E0;
  SetForeColour(blue + green);
  generate_swi_interrupt(); // Function defined in interrupts.s
}
*/

/* Initializes the Operating System. The interruptions have to be disabled at entrance.
*
*
*  - Sets interruptions
*  - Sets the periodic timer
*  - Set the thread functionality
*
*  This function is called by the main() function defined in arm_asm/start.s file.
*/
void init() {

  /* Initializes ourselves as a thread so we can use locks,
    then enable console locking. */
  thread_init();

  /* Initializes the frame buffer and console. */
  framebuffer_init();
  video_init();

  printf("\nosOs Kernel Initializing");

  /* Initialize memory system. */
  palloc_init (user_page_limit);
  malloc_init ();

  /* Initializes the Interrupt System. */
  interrupts_init();
  timer_init();

  timer_msleep(5000000);

  /* Starts preemptive thread scheduling by enabling interrupts. */
  thread_start();
  serial_init_queue ();

  printf("\nFinish booting.");

  init_all_threads();

  int i = 0;
  while(i < 10) {
	  i++;
      enum interrupts_level old_level = interrupts_disable();
      unsigned short red = 0xF800;
      unsigned short green = 0x7E0;
      SetForeColour(red + green);
      printf("\nosOs v0.0 Forever: ");
      printf(" Thread: %s", thread_current()->name);
      printf(", Priority: %d", thread_current()->priority);
      interrupts_set_level(old_level);
  }

  thread_exit ();
}

static void init_all_threads() {
  lock_init(&lock_task);
  thread_create("Thread 0", PRI_MAX, &task_0, NULL);
  thread_create("Thread 1", PRI_MAX, &task_1, NULL);
  thread_create("Thread 2", PRI_MAX, &task_2, NULL);
  thread_create("Thread 3", PRI_MAX, &task_3, NULL);
  thread_create("Thread 4", PRI_MAX, &task_4, NULL);
  thread_create("Thread 5", PRI_MAX, &task_5, NULL);
  thread_create("Thread 6", PRI_MAX, &task_6, NULL);
}

/* Task 1 prints the numbers from 0 to 50. */
static void task_0(void *param) {
  unsigned short green = 0x7E0;
  int i = 0;
  while (i < 50) {
      SetForeColour(green);
      printf("\n%s %d - Counting %d", thread_current()->name, thread_current()->tid, i++);
  }
}

/* Task 1 prints all numbers. */
static void task_1(void *param) {
  unsigned short blue = 0x1f;
  unsigned short green = 0x7E0;
  int i = 0;
  while (i < 3) {
	  i++;
      int32_t x =  434343334;
      int32_t y = 333443433;
      int32_t z = x / y;
      SetForeColour(blue + green);
      printf("\n%s - Diving long numbers %d / %d = %d", thread_current()->name, x, y, z);
  }
}

/* Task 2 Generates Random numbers */
static void task_2(void *param) {
  unsigned short green = 0x7E0;
  SetForeColour(green);
  printf("\nTrying to acquired lock: %s", thread_current()->name);

  lock_acquire(&lock_task);
  int i = 0;
  while (i++ < 10) {
      SetForeColour(green);
      printf("\nLock Acquired by %s - Generating Random Num: %d", thread_current()->name,
          (int) random_ulong());
  }
  lock_release(&lock_task);
}

/* Task 3 Greets USF. */
static void task_3(void *param) {
  unsigned short color = 0x5659;
  uint32_t *ptr = (uint32_t *) 0x20000;

  int i = 0;
  while (i++ < 5) {
      SetForeColour(color);
      printf("\n%s, - Address [%d] = %d",  thread_current()->name, (uint32_t) ptr, *ptr);
      ptr += 1;
  }
}

/* Task 4 prints 1 to 10. */
static void task_4(void *param) {
  unsigned short red = 0xF800;
  unsigned short blue = 0x1f;
  unsigned short green = 0x7E0;

  printf("\nTrying to acquired lock: %s", thread_current()->name);

  lock_acquire(&lock_task);
  int i = 1;
  while (i++ < 15) {
    SetForeColour(blue + green + red);
    printf("\nLock acquired by %s - Value %d", thread_current()->name, i);
  }
  lock_release(&lock_task);
}

/* Task 4 Blinking the ACK led */
static void task_5(void *param) {
  unsigned short red = 0xF800;

  unsigned int gpio_register = 16;
  unsigned int function = 1;
  gpio_enable_function(gpio_register, function);

  int i = 1;
  while (i++ < 20) {
      SetForeColour(red);
      printf("\n%s - Blinking ACK led (Absolutely awesome)", thread_current()->name);
      gpio_set_register(16, 0);
      timer_msleep(20000);
      gpio_set_register(16, 1);
      timer_msleep(20000);
  }
}

static int factorial(int number) {
  if (number == 0 || number == 1) {
      return 1;
  }

  return number * factorial (number - 1);
}

/* Task 6 calculates the factorial. */
static void task_6(void *param) {
  unsigned short blue = 0x1f;
  unsigned short green = 0x7E0;

  int i = 1;
  while (i++ < 250) {
      int number = i % 25;
      int fac1 = factorial(number);
      int fac2 = factorial(number);

      ASSERT(fac1 == fac2);
      SetForeColour(green + blue);
      printf("\n%s - Factorial(%d) = %d", thread_current()->name, number, fac1);
  }
}

