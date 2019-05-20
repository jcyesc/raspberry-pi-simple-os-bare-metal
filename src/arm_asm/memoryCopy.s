/******************************************************************************
*	memoryCopy.s
*
*	memoryCopy.s contains functions to perform memory copies.
*
******************************************************************************/

.section .text

/*
 * Copies "size" bytes from src to dest. It does this by first copying consecutive addresses of
 * memory into 10 registers (r3 to r12), in this way, during every iteration 40 bytes are being
 * copy (10 words * 4 bytes = 40). If the number of bytes to copy is not multiple of 40, the rest
 * is copied word by word and byte by byte.
 *
 * Note: If the separation between src and dest is less than 40 bytes, there could be OVERLAPPING.
 *
 * void memory_fastest_copy(char *src, char *dest, int size)
 */
.globl memory_fastest_copy
memory_fastest_copy:
	stmfd sp!, {r4-r12,lr}			// Saving the stack frame.

	src .req r0			// Source address
	dest .req r1		// Destination address
	size .req r2		// # of bytes to copy

	// Copies 10 words at a time (40 bytes).
	// We're going to use r3 through r12, so block size is 10 words = 40 bytes
	memory_fastest_copy_while_10_words$:
		cmp size, #40				// four bytes per word, 10 word registers = 40 bytes copied in each iteration.
		bls memory_fastest_copy_exit_10_words$
		subs size, #40  			// four bytes per word, 10 word registers = 40 bytes copied in each iteration.
		ldmia src!, {r3-r12}		// Loading 10 words at a time (Load - Full Ascending Stack - Increase After).
		stmia dest!, {r3-r12}		// Storing 10 words at a time (Store - Full Ascending Stack - Increase After).
		b memory_fastest_copy_while_10_words$	// Repeating the loop.
	memory_fastest_copy_exit_10_words$:

	// Copies 1 word at a time (4 bytes).
	memory_fastest_copy_while_1_word$:
		cmp size, #4				// Making sure that the number of bytes to copy are more or equals to 4 bytes.
		bls memory_fastest_copy_exit_1_word$
		subs size, #4				// Subtracting the number of bytes copied.
		ldmia src!, {r3}			// Loading 1 word at a time (Load - Full Ascending Stack - Increase After).
		stmia dest!, {r3}			// Storing 1 word at a time (Store - Full Ascending Stack - Increase After).
		b memory_fastest_copy_while_1_word$		// Repeating the loop.
	memory_fastest_copy_exit_1_word$:

	// Copies 1 byte at a time.
	memory_fastest_copy_while_1_byte$:
		cmp size, #1
		bls memory_fastest_copy_exit_1_byte$
		subs size, #1				// Subtracting the number of bytes copied.
		ldrb r3, [src], #1			// Post index mem[base] -> base = base + offset (ldrb r0, [r1] #4)
		strb r3, [dest], #1			// Post index mem[base] -> base = base + offset (strb r0, [r1] #4)
		b memory_fastest_copy_while_1_byte$
	memory_fastest_copy_exit_1_byte$:

	.unreq src
	.unreq dest
	.unreq size
	ldmfd sp!, {r4-r12,pc}		// Restoring the stack frame.

/* End of memory_fastest_copy. */


/* Copies length words from src to dest and returns the remaining bytes that were not copied. This
 * method copies 40 bytes at a time. If the number of bytes is less than 40, it won't do any copy.
 *
 * Note: If the separation between src and dest is less than 40 bytes, there could be OVERLAPPING.
 * int memory_fast_copy(char *src, char *dest, int length)
 */
.globl memory_fast_copy
memory_fast_copy:
	stmfd sp!, {r4-r12,lr}			// Saving the stack frame.

	src .req r0					// Defining variables to be used.
	dest .req r1
	length .req r2
	// We're going to use r3 through r12, so block size is 10 words = 40 bytes
	memory_fast_copy_while$:
		subs length, #40  // four bytes per word, 10 word registers
		bls memory_fast_copy_exit$
		ldmia src!, {r3-r12}		// Loads 10 words from source in r3-r12
		stmia dest!, {r3-r12}		// Stores 10 words in dest from r3-r12
		b memory_fast_copy_while$
	memory_fast_copy_exit$:
		mov r0, length				// Return the remaing bytes to copy.

	.unreq src
	.unreq dest
	.unreq length
	ldmfd sp!, {r4-r12,pc}			// Restoring the stack frame.
/* Ends of memory_fast_copy. */

