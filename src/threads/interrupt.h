/*
 * interrupts.h
 *
 * Note: By the default the FIQ interruptions will be disabled.
 */

#ifndef THREADS_INTERRUPTS_H_
#define THREADS_INTERRUPTS_H_

#include <stdbool.h>
#include <stdint.h>

/* Enum that defines the status of the interrupts: ON or OFF. */
enum interrupts_level {
  INTERRUPTS_OFF,       /* Interrupts disabled (IRQ and FIQ). */
  INTERRUPTS_ON         /* Interrupts enabled (IRQ and FIQ). */
};

/* Interrupt stack frame*/
struct interrupts_stack_frame {
  /* Push by irq_handler_int in interruptsHandlers.s.
     These are the interrupted task's saved registers.
     This is the stack frame of the USER's mode. */
  uint32_t cpsr;             /* Save cpsr (Current Process Status Register for the USER's task.) */
  uint32_t *r13_sp;         /* Save r13 Change to SYS MODE to get the USER's SP) */
  uint32_t *r14_lr;         /* Save r14 Change to SYS MODE to get the USER's LR) */
  uint32_t *r15_pc;         /* Save r15 USER's PC is the LR_irq register. */
  uint32_t r0;               /* Save r0 */
  uint32_t r1;               /* Save r1 */
  uint32_t r2;               /* Save r2 */
  uint32_t r3;               /* Save r3 */
  uint32_t r4;               /* Save r4 */
  uint32_t r5;               /* Save r5 */
  uint32_t r6;               /* Save r6 */
  uint32_t r7;               /* Save r7 */
  uint32_t r8;               /* Save r8 */
  uint32_t r9;               /* Save r9 */
  uint32_t r10;              /* Save r10 */
  uint32_t r11;              /* Save r11 */
  uint32_t r12;              /* Save r12 */
};

/* Signature of the interrupt handler function.*/
typedef void interrupts_handler_function(struct interrupts_stack_frame *);

/* Initializes the interrupt system.*/
void interrupts_init(void);

/* Register the IRQ handler for the given interrupt number. The BCM2835 has 64 IRQ interruptions
 * that are enumerated from 0 to 63.
 */
void interrupts_register_irq(unsigned char interrupt_number, interrupts_handler_function *,
    const char *name);

/* Register the SWI hander for the Software interrupts. */
void interrupts_register_swi(interrupts_handler_function *, const char *name);

/* Return the IRQ name that correspond to the interrupt number. */
const char* interrupts_get_irq_name(unsigned char interrupt_number);

/* Return the SWI name. */
const char* interrupts_get_swi_name();

/* Returns the interrupt level. */
enum interrupts_level interrupts_get_level(void);

/* Sets the interrupts level and returns the previous one. */
enum interrupts_level interrupts_set_level(enum interrupts_level);

/* Enables the interrupts and returns the previous one. */
enum interrupts_level interrupts_enable(void);

/* Disables the interrupts and returns the previous one. */
enum interrupts_level interrupts_disable(void);

/* Prints status of the interrupts. */
void interrupts_print_status(void);

/* Returns true during processing of an external interrupt and false at all other times. */
bool interrupts_context(void);
/* Returns true if an IRQ was generated. Otherwise is false. */
bool interrupts_was_irq_generated(void);

/* During processing of an external interrupt, directs the
   interrupt handler to yield to a new process just before
   returning from the interrupt.  May not be called at any other
   time. */
void interrupts_yield_on_return (void);

/* SWI: Software Interrupt Handler. */
void interrupts_dispatch_swi(struct interrupts_stack_frame *stack_frame, int32_t swi_number);

/* IRQ: Interrupt Request Handler. */
void interrupts_dispatch_irq(struct interrupts_stack_frame *stack_frame) ;

void interrupts_debug(struct interrupts_stack_frame *stack_frame);

#endif /* THREADS_INTERRUPTS_H_ */
