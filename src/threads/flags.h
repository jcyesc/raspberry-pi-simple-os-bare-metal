
#ifndef THREADS_FLAGS_H_
#define THREADS_FLAGS_H_

#define FLAG_FIQ        0x40            /* Fast Interrupt Request Flag (FIQ) */
#define FLAG_IRQ        0x80            /* Interrupt Request Flag (IRQ) */

/******************************************************************************
 * Processor modes
 ******************************************************************************/
#define USER_MODE 0x10
#define FIQ_MODE 0x11
#define IRQ_MODE 0x12
#define SVC_MODE 0x13
#define ABT_MODE 0x17
#define UND_MODE 0X1b
#define SYS_MODE 0x1f

#endif /* THREADS_FLAGS_H_ */
