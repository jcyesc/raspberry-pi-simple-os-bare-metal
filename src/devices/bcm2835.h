/*
 * bcm2835.h
 *
 * BCM2835
 *
 * The BCM2835 is a Soc (System-on-a-chip) designed by Broadcom. It is used on the Raspberry Pi,
 * where it is easily visible as the black chip in the center of the board. The chip contains many
 * of the components of a traditional computer, such as a CPU, memory, and a GPU.
 *
 * The BCM2835 is actually not specific to the Raspberry Pi and is used in at least one other
 * consumer device.
 *
 * This file contains the constants that define where the Peripherals, Interrupt controllers, mail box registers
 * UART registers are in memory.
 */


#ifndef DEVICES_BCM2835_H_
#define DEVICES_BCM2835_H_

/**************************************************************************************
 * ARM physical memory mapping of BCM2835 peripherals                                 *
 *                                                                                    *
 * Physical addresses range from 0x20000000 to 0x20FFFFFF for peripherals.            *
 **************************************************************************************/

/* Start of memory-mapped peripherals address space */
#define PERIPHERALS_BASE 0x20000000  // 0x20000000 = 536870912 = 512 MB

/* System timer */
#define SYSTEM_TIMER_REGISTERS_BASE (PERIPHERALS_BASE + 0x3000)

/* Interrupt controller (for ARM) */
#define INTERRUPT_CONTROLLER_BASE (PERIPHERALS_BASE + 0xB000)

/* Interrupt controller (for ARM) -  Pending IRQs in the range 0-31 */
#define INTERRUPT_REGISTER_PENDING_IRQ_0_31 (INTERRUPT_CONTROLLER_BASE + 0x204)

/* Interrupt controller (for ARM) -  Pending IRQs in the range 32-63 */
#define INTERRUPT_REGISTER_PENDING_IRQ_32_63 (INTERRUPT_CONTROLLER_BASE + 0x208)

/* Interrupt controller (for ARM) -  Enable IRQs in the range 0-31 */
#define INTERRUPT_REGISTER_ENABLE_IRQ_0_31 (INTERRUPT_CONTROLLER_BASE + 0x210)

/* Interrupt controller (for ARM) -  Enable IRQs in the range 32-63 */
#define INTERRUPT_REGISTER_ENABLE_IRQ_32_63 (INTERRUPT_CONTROLLER_BASE + 0x214)

/* Mailbox */
#define MAILBOX_REGISTERS_BASE (PERIPHERALS_BASE + 0xB880)

/* Power management / watchdog timer */
#define PM_REGISTERS_BASE (PERIPHERALS_BASE + 0x100000)

/* PL011 UART */
#define PL011_REGISTERS_BASE (PERIPHERALS_BASE + 0x201000)

/* SD host controller */
#define SDHCI_REGISTERS_BASE (PERIPHERALS_BASE + 0x300000)


/***************************************************************************
* IRQ lines of selected BCM2835 peripherals. Note about the numbering      *
* used here: IRQs 0-63 are those shared between the GPU and CPU, whereas   *
* IRQs 64+ are CPU-specific.                                               *
***************************************************************************/

/* IRQ linnes*/
/* System timer - one IRQ line per output compare register. */
#define IRQ_0 0         // IRQ line for the system timer compare register 0 (used by the GPU)
#define IRQ_1 1         // IRQ line for the system timer compare register 1
#define IRQ_2 2         // IRQ line for the system timer compare register 2 (used by the GPU)
#define IRQ_3 3         // IRQ line for the system timer compare register 3

#endif /* DEVICES_BCM2835_H_ */
