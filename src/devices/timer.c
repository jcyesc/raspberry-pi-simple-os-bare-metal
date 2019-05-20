
#include <stdio.h>

#include "timer.h"
#include "bcm2835.h"
#include "../threads/interrupt.h"
#include "../threads/thread.h"

#define TIMER_PERIODIC_INTERVAL 500000 // Time in miliseconds

struct bcm2835_system_timer_registers {
  volatile unsigned int CS;  /** System Timer Control/Status */
  volatile unsigned int CLO; /** System Timer Counter Lower 32 bits */
  volatile unsigned int CHI; /** System Timer Counter Higher 32 bits */
  volatile unsigned int C0;  /** System Timer Compare 0.  DO NOT USE; is used by GPU.  */
  volatile unsigned int C1;  /** System Timer Compare 1 */
  volatile unsigned int C2;  /** System Timer Compare 2.  DO NOT USE; is used by GPU.  */
  volatile unsigned int C3;  /** System Timer Compare 3 */
};

/* Pointer to the timer registers. */
static volatile struct bcm2835_system_timer_registers * const timer_registers =
        (volatile struct bcm2835_system_timer_registers*) SYSTEM_TIMER_REGISTERS_BASE;

/* Timer interrupt handler. */
static void timer_irq_handler(struct interrupts_stack_frame *stack_frame);

/* Resets the System Timer Compare register (C0-C3) )in the Timer Control/Status register. */
static void timer_reset_timer_compare(int timer_compare);

/* Sets the periodic interval of the timer. */
static void timer_set_interval(int timer_compare, int milliseconds);

void timer_init() {
  printf("\nInitializing timer.....");
  interrupts_register_irq(IRQ_1, timer_irq_handler, "Timer Interrupt");
  timer_set_interval(IRQ_1, TIMER_PERIODIC_INTERVAL);
}

/* Returns the timestamp. */
int timer_get_timestamp() {
  /* Assigning the higher part of the timestamp.*/
  //unsigned long long timestamp = timer_registers->CHI;
  // Shifting the higher part to the right and accomodating the lower part from the 0 bit to 31.
  //timestamp = (timestamp << 32) | timer_registers->CLO;

  return timer_registers->CLO;
}

// TODO support 64 bits values
void timer_msleep(int milliseconds) {
  // Implements busy waiting
  int startTime = timer_get_timestamp();
  int elapseTime = timer_get_timestamp() - startTime;
  while (milliseconds > elapseTime) {
      elapseTime = timer_get_timestamp() - startTime;
  }
}

/* Resets the System Timer Compare register (C0-C3) )in the Timer Control/Status register.
 * After a timer interrupt, to clear the interrupt, the software must write 1 to the bit in CS that
 * has the index the same as that of the System Timer Compare register. That is, to clear an
 * interrupt set in C1, software must write 0x20 to CS, and to clear an interrupt set iN C3,
 * software must write 0x80 to CS.
 */
static void timer_reset_timer_compare(int timer_compare) {
  // There are only four system timer compares (0-3).
  if (timer_compare > 3 || timer_compare <0) {
      return;
  }

  timer_registers->CS = timer_registers->CS | (1 << timer_compare);
}

/* Timer interrupt handler.
 * To receive the scheduled interrupt, the software must have previously enabled the corresponding
 * IRQ line using the BCM2835 interrupt controller.
 */
static void timer_irq_handler(struct interrupts_stack_frame *stack_frame) {
  printf("\nKernel - Timer Interrupt Handler.");

  // The System Timer compare has to be reseted after the timer interrupt.
  timer_reset_timer_compare(IRQ_1);

  thread_tick(stack_frame);

  //timer_msleep(1000000);
  timer_msleep(300000);

  // The System Timer compare register has to be set up with the new time after the timer interrupt.
  timer_set_interval(IRQ_1, TIMER_PERIODIC_INTERVAL);
}

/* Sets the periodic interval of the timer.
 * Set the timestamp value in the BCM2835 System timer. The interface to the BCM2835 System Timer
 * is a set of 32-bit memory-mapped registers.
 */
static void timer_set_interval(int timer_compare, int milliseconds) {
  int interval = timer_registers->CLO + milliseconds;
  // The System Timer Compares that are enabled are 1 and 3.
  if (timer_compare == 1) {
      timer_registers->C1 = interval;
  } else if (timer_compare == 3) {
      timer_registers->C3 = interval;
  }
}
