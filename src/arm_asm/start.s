/******************************************************************************
*	main.s
*
*   This is the entry point  for the Operating system.
*   This files contains the functions that boot the system and configure the exceptions.
*
******************************************************************************/

/*
* This section will be placed in the address 0x8000.
*/
.section .init

/*
* Configuring Exception Handlers.
*/
.globl _start
_start:
    ldr pc, reset_handler			// Reset exception. Note that is the first instruction to be executed.
    ldr pc, undefined_handler		// Undefined instructions
    ldr pc, swi_handler				// Software Interrupt (SWI)
    ldr pc, prefetch_handler		// Prefetch abort
    ldr pc, data_handler			// Data abort
    ldr pc, unused_handler			// Not assigned
    ldr pc, irq_handler				// IRQ
    ldr pc, fiq_handler				// FIQ
reset_handler:      .word reset
undefined_handler:  .word hang
swi_handler:        .word swi_handler_int		// swi_handler_int() is defined in interruptsHandlers.s
prefetch_handler:   .word hang
data_handler:       .word hang
unused_handler:     .word hang
irq_handler:        .word irq_handler_int
fiq_handler:        .word hang

/*******************************************************************************************
* reset() function
*
* The reset function copies the interruption vector that is at address 0x8000 to
* the address 0x0000 where the ARM processor expects it to be.
*
* Once that the interruption vector is copied, the main function is called.
********************************************************************************************/
reset:
/* Set the interrupt vector. */
    mov r0, #0x8000
    mov r1, #0x0000
    ldmia r0!, {r2,r3,r4,r5,r6,r7,r8,r9}
    stmia r1!, {r2,r3,r4,r5,r6,r7,r8,r9}
    ldmia r0!, {r2,r3,r4,r5,r6,r7,r8,r9}
    stmia r1!, {r2,r3,r4,r5,r6,r7,r8,r9}

/* Set stack for the IRQ mode and disable the FIQ and IRQ interrupts. */
    mov r0, #0xD2				// Disabling FIQ, IRQ and setting the IRQ Mode.
    msr cpsr_c, r0
    mov sp, #0x8000         		// Setting the stack for IRQ Mode.

/*
* Set stack for the SVC mode and disable the FIQ and IRQ interrupts.
*
* When the Operating System takes control, the Interruptions (IRQ and FIQ)
* have to be disabled.
*/
    mov r0, #0xDF				// Disabling FIQ, IRQ and setting the System Mode.
    msr cpsr_c, r0
    mov sp, #0x40000	  		    // Setting the stack for SVC Mode.
    sub sp, #0x4					// (The botton of the stack is at a page boundary).
    								// This is done to initialize the kernel as a thread. See
    								// thread_init() for more info. We are assuming that the kernel
    								// code is below #Ox40000
    								// The stack grows downwards and it is a full descending one.
    bl main

/************************************************************************************************
* MEMORY ALLOCATION
************************************************************************************************
Note:
The memory that is allocated via palloc or malloc starts in the address:
	0x20000 and ends in the address 0x20000000. The address 0x20000000 is not included.

    Total memory = 0x20000000 - 0x40000 = 0x1FFC0000 = 536,739,840 ~= 512MB
************************************************************************************************/

/********************************************************************************************
* Hang function
********************************************************************************************/
hang: b hang


/********************************************************************************************
* main() function
*
* main is what we shall call our main operating system method. It never 
* returns, and takes no parameters.
* C++ Signature: void main(void)
*********************************************************************************************/
main:

	/* Initializes the kernel (enable IRQ, sets the periodic timer). */
	bl init


