/************************************************************************************
*	interruptsHandlers.s
*
*	Defines the handlers for the different kind of interruptions. It also
*   provides the functions to enable and disable the interruptions: IRQ and FIQ.
*
*************************************************************************************/

.section .data

.align 4
.globl exec_int_msg
exec_int_msg:
  		.asciz "\n!!!!!!!!!!!!! Executing SWI interrupt !!!!!!!!!!!!!!!"
// .asciz ends the string with null ('\0').

.section .text

/* Prints a message indicating that an interrupt is being executed.
*  void print_executing_interrupt()
*/
.globl print_executing_interrupt
print_executing_interrupt:
	push {lr}
	ldr   r0, =exec_int_msg
	bl printf
	pop {pc}

/* Generates a Software Interrupt.
* void generate_swi_interrupt()
*/
.globl generate_swi_interrupt
generate_swi_interrupt:
	push {lr}
	//bl print_executing_interrupt
	swi 0x11
	pop {pc}


/* Handles the software interrupts. It extracts the SWI number an pass it to the handler
* written in C. It accesses to the parameter through the LR register.
* void swi_handler_int()
*/
.globl swi_handler_int
swi_handler_int:
	mov r0, #0x5
	mov r12, #0xc

	stmfd sp!, {r0-r12,lr}			// Saving context

	// Store CPSR and SPSR in the stack.
	mrs r0, cpsr
	mrs r1, spsr
	stmfd sp!, {r0,r1}				// Store CPSR and SPSR in the stack.

	// Call the SWI dispatcher with the Stack Frame and the SWI number.
	mov r0, sp						// Passing the Stack Frame address to the function.
	ldr r1, [lr, #-4]				// Read the SWI instruction to extract the parameter
	bic r1, r1, #0xff000000 	 	// Mask off top 8 bits to extract the parameter.
	bl interrupts_dispatch_swi		// Calling the SWI handler that is defined inn interrupts.c.

	// Restoring CPSR and SPSR from the stack.
	ldmfd sp!, {r0,r1}				// Restoring CPSR and SPSR from the stack.
   	msr cpsr, r0
  	msr spsr, r1

	ldmfd sp!, {r0-r12,pc}^				// After the exception, it is necessary to return to the
									// address pointed by LR.
									// (ARM System Developer Guides, 1st edition, pg 323)


/* Handles the IRQ interrupts.
*
* When this method is executed the MODE is IRQ and the the previous MODE has to be USER's mode.
* The processing status of the USER's mode is saved in SPRS.
*
* Signature void irq_handler_int()
*/
.globl irq_handler_int
irq_handler_int:
	sub lr, lr, #4					// Making sure lr points to the right return address.
									// This is lr_irq or pc_user.
	stmfd sp!, {r0-r12}			    // Saving context (r0-r12).

	/* Change to SYS Mode to save the SP_USR and LR_USR. */
	mov r0, #0xdf		// (SYS_MODE + NO_INT = 0x1f + 0xc = 0xdf)
	msr cpsr_c, r0
	mov r1, sp			// Setting the USER's SP.
	mov r2, lr			// Setting the USER's LR.
	mov r0, #0xd2		// Changing mode to IRQ with interrups disable.
	msr cpsr_c, r0

	stmfd sp!, {r1,r2,lr}	// Saving context (sp_usr, lr_usr, pc_usr).

	// Store SPSR (USER's CPSR) in the stack.
	mrs r0, spsr
	stmfd sp!, {r0}				// Store SPSR (USER's CPSR) in the stack.

	// Calling the interrupt handler
	mov r0, sp						// Passing the Stack Frame address to the function.
	bl interrupts_dispatch_irq		// Calling the IRQ handler that is defined in interrupts.c.

	// Restoring SPSR (USER's CPSR) from the stack.
	ldmfd sp!, {r0}				// Restoring SPSR (USER's CPSR) from the stack.
	msr spsr, r0

	// lr here is referencing to lr_irq.
	ldmfd sp!, {r1,r2,lr}		// Restoring (sp_usr, lr_usr, pc_usr)

	/* Setting the sp_usr and lr_usr in the SYS MODE. */
	mov r0, #0xdf		// (SYS_MODE + NO_INT = 0x1f + 0xc = 0xdf)
	msr cpsr_c, r0
	mov sp, r1			// Restoring the USER's SP.
	mov lr, r2			// Restoring the USER's LR.
	mov r0, #0xd2		// Changing mode to IRQ with interrups disable.
	msr cpsr_c, r0

	// Restoring all the registers from the Stack frame.
	ldmfd sp!, {r0-r12}		// After the exception, it is necessary to return to the instruction
	movs pc, lr				// that was being executed before the interruption.

/* Returns the CPSR status register.
*
* Signature:	int get_cpsr_value(void)
*/
.globl get_cpsr_value
get_cpsr_value:
	mrs r0, cpsr			// Sets the CPSR value in r0;
	mov pc, lr				// Returning to the caller


/**************************************************************
* Enable and disable interruptions: IRQ and FIQ               *
***************************************************************/

/*
* Enable IRQ interruptions
*
* PRE		nzcvqjIFt_SVC
* POST		nzcvqjiFt_SVC
*
* Signature:	void enable_irq_interruptions(void)
*/
.globl enable_irq_interruptions
enable_irq_interruptions:
	mrs r1, cpsr
   	bic r1, r1, #0x80		// Enable IRQ interruptions ( The I bit (8 bit) has to be 0)
   	msr cpsr_c, r1
	mov pc, lr				// Returning to the caller.


/*
* Disable IRQ interruptions
*
* PRE		nzcvqjift_SVC
* POST		nzcvqjIft_SVC
*
* Signature:	void disable_irq_interruptions(void)
*/
.globl disable_irq_interruptions
disable_irq_interruptions:
	mrs r1, cpsr
    orr r1, r1, #0x80		// Disable IRQ interruptions ( The I bit (8 bit) has to be 1)
    msr cpsr, r1
    mov pc, lr				// Returning to the caller.


/* Enable FIQ interruptions
*
* PRE		nzcvqjIFt_SVC
* POST		nzcvqjIft_SVC
*
* Signature:	void enable_fiq_interruptions(void)
*/
.globl enable_fiq_interruptions
enable_fiq_interruptions:
	mrs r1, cpsr
   	bic r1, r1, #0x40		// Enable FIQ interruptions ( The f bit (7 bit) has to be 0)
   	msr cpsr_c, r1
	mov pc, lr				// Returning to the caller.


/*
* Disable FIQ interruptions
*
* PRE		nzcvqjift_SVC
* POST		nzcvqjiFt_SVC
*
* Signature:	void disable_fiq_interruptions(void)
*/
.globl disable_fiq_interruptions
disable_fiq_interruptions:
	mrs r1, cpsr
    orr r1, r1, #0x40		// Disable FIQ interruptions ( The f bit (7 bit) has to be 1)
    msr cpsr, r1
    mov pc, lr				// Returning to the caller.


/*
* Get the value of the current sp.
*
* Signature:	void * get_current_sp(void)
*/
.globl get_current_sp
get_current_sp:
	mov r0, sp		// Returning the stack pointer.
	mov pc, lr		// Returning to the caller.



