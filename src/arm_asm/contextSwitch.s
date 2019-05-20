/************************************************************************************
*	contextSwitch.s
*
*	Defines the functions that are in charge of executing a context switch.
*
*************************************************************************************/

/**
* This method was created for testing purposes. It makes sure that the data from the registers
* is stored in the interrupts_stack_frame.
*
* Signature: void save_and_switch_context(struct interrupts_stack_frame *cur_stack_frame);
*/
.globl save_context
save_context:
	push {r4-r12} 		/* Saves the r4 to r12 registers in order to modify them. */

	mov r1, #1
	mov r2, #2
	mov r3, #3
	mov r4, #4
	mov r5, #5
	mov r6, #6
	mov r7, #7
	mov r8, #8
	mov r9, #9
	mov r10, #10
	mov r11, #11
	mov r12, #12

	push {r1}			// Saves r1 temporaly (*next_stack_frame).
	/* Store CPSR in the interrupts_stack_frame. */
	mrs r1, cpsr
	stmea r0!, {r1}
	pop {r1}

	// Stores the values of the registers in the interrupt stack frame.
	stmea r0!, {sp}
	stmea r0!, {lr}
	stmea r0!, {lr}
	stmea r0!, {r0-r12}

	pop {r4-r12}	/* Restores the r4 to r12 registers. */
	mov pc, lr		/* Returns to the caller. */


/* Saves the context of the previous thread and makes a context switch.
*
*  The steps executed by this function are:
*
*    - 1. Save the stack frame or context of the previous thread.
*    - 2. Call thread_schedule_tail(previous, next).
*    - 3. Execute a context switch using the stack frame of the next thread to run.
*
* Signature: void save_and_switch_context(struct interrupts_stack_frame *cur_stack_frame,
*     struct interrupts_stack_frame *next_stack_frame, struct thread *cur, struct thread *next);
*/
.globl save_and_switch_context
save_and_switch_context:
	/*******************************************************************************************
	 * 1. Saves the context of previous thread.
	 *******************************************************************************************/

	push {r1}					// Saves the value of r1 (*next_stack_frame pointer) temporaly.
	/* Store CPSR in the interrupts_stack_frame. */
	mrs r1, cpsr				// Saves the value of CPSR.
	stmea r0!, {r1}
	pop {r1}					// Restores the value of r1 (*next_stack_frame pointer).
	/* Saves context (sp_usr, lr_usr, pc_usr). (In this case, the PC is equals to the LR due we want
	to return from this function when the current thread is scheduled again.) */
	stmea r0!, {sp}				// Saves the value of SP
	stmea r0!, {lr}				// Saves the value of LR
	stmea r0!, {lr}				/* Saves the value of LR in the PC because this is going to be
									the next instruction to be executed. */
	stmea r0!, {r0-r12}			// Saves context (r0-r12).
	/* Note: The content of r0-r3 doesn't matter in this case. It is also important to notice
	that if we save the values of r0-r3, this might represent a security threat because when the
	thread is scheduled again, it might modify information of other thread. */

	/*******************************************************************************************
	 * 2. Call thread_schedule_tail(previous, next).
	 ********************************************************************************************/

	/* It doesn't matter that the LR register is not saved because we are not returning from the
	save_and_switch_context() function. */

	/* It is necessary to change from SYS_MODE to IRQ_MODE because when thread_schedule_tail() is
	   executed, if the current thread is DYING, then the page that was assigned to the thread will
	   be destroyed and the SP will be no longer valid. */
	mov r4, #0xd2				// Selects the mode to IRQ with interrups disable
	msr cpsr_c, r4				// Sets the IRQ mode.

	push {r1}					// Saves next stack frame pointer.

	mov r0, r2					// Passes thread *cur as the first parameter.
	mov r1, r3					// Passes thread *next as the second parameter
	bl thread_schedule_tail		// Calls the function thread_schedule_tail(struct thread *current, struct thread *next)

	pop {r1}					// Restores next stack frame pointer.

	/********************************************************************************************
	 * 3. Execute a context switch using the stack frame of the next thread to run.
	 ********************************************************************************************/

	/* This code is running in IRQ_MODE. */
	ldmfd r1!, {r0}				// Restores the CPSR from the stack frame.
	msr spsr, r0				// Sets the SPSR of the IRQ Mode

	// LR here is referencing to lr_irq.
	ldmfd r1!, {r2,r3,lr}		// Restoring (SP_usr, LR_usr, PC_usr)

	mov r0, #0xdf				// (SYS_MODE + NO_INT = 0x1f + 0xc = 0xdf)
	msr cpsr_c, r0				// Sets the mode to SYSTEM MODE with the interrupts disable.
	mov sp, r2					// Sets the SP of the SYSTEM MODE.
	mov lr, r3					// Sets the LR of the SYSTEM MODE.
	mov r0, #0xd2				// Selects the mode to IRQ with interrupts disable.
	msr cpsr_c, r0				// Sets the mode to IRQ with interrups disable.

	// Restoring all the registers from the Stack frame.
	ldmfd r1!, {r0}				// Restoring r0 from the interrupts_stack_frame.
	push {r0}					// Stores r0 temporaly in the SP_IRQ.
	mov r0, r1
	ldmfd r0!, {r1-r12}			// Loads the registers (r1-r12) with the values in the interrupts_stack_frame.
	pop {r0}					// Restores r0 from the SP_IRQ.

	movs pc, lr					// Change to SYSTEM MODE and sets the PC the value in LR_IRQ.


