
# Raspberry PI Simple OS Bare metal

This project has the purpose to show how a simple operating system works and the different 
parts that need to be configured.

This Operating System runs in the following Raspberry PIs:

1. Raspberry Pi Model A (Soc Broadcom BCM2835, CPU ARM1176JZF-S 700 MHz)
2. Raspberry Pi Model B (Soc Broadcom BCM2835, CPU ARM1176JZF-S 700 MHz)

# Requirements

In order to execute the Raspberry PI Simple OS Bare metal we need to:

- Install yagarto's toolchain
- Firmware files provided by the Raspberry PI project (`bootcode.bin`, `start.elf` and `config.txt`). This
files can be found in directory backup-boot.

## Download and Install yagarto

You can download yagarto toolchain [here](https://sourceforge.net/projects/yagarto/).

Make sure to read the `yagarto's readme file` before installing, so you can configure the `arm-none-eabi` in 
the $PATH environment variable.

More information [here](https://www.cl.cam.ac.uk/projects/raspberrypi/tutorials/os/downloads.html)

# Features currently supported

The Raspberry PI Simple OS Bare Metal implements thread features, simple memory management,
interrupts configuration, basic libraries and output to the console using HDMI.

## Thread features

1. Creation of threads
2. Context Swich implementation
3. Process scheduler (Round Robin)
4. Every thread has its own stack
5. Only Kernel threads are supported and they run in SYSTEM_MODE
6. Timer Tick supported
7. Implement the methods:

	thread_current()
	thread_get_running_thread()
	thread_create()
	thread_block()
	thread_unblock()
	thread_name()
	thread_tid()
	thread_yield()
	schedule()

8. Idle thread functionality is supported.

## Memory system features

1. Implements malloc.h (memory allocator)
2. Implements palloc.h (page allocator, use during thread creation)
3. Doesn't support MMU (Memory Management unit)

## Screen support through HDMI

1. Initialize the frame buffer.
2. Implements devices/video.h to support the output of ASCII characters in the screen
3. Implements lib/kernel/console.h to make sure that the messages are displayed in full (use locks)

## GPIO

1.  Implements devices/gpio.h to support output through leds.

## Synchronization

1. Implements semaphores
2. Implements locks
3. Implements conditions variables

## Interruptions

1. Configure the interruptions
2. Configure timer interruption
3. Configure software interruption

## Debug and assertions

1. Implements lib/debug.h to debug and assert conditions in the kernel.

## Supports floating point arithmetic

In order to support the module and division operations, the `libgcc.a` is used. The library is part of yagarto
cross compiler (yagarto-4.7.2/lib/gcc/arm-none-eabi/4.7.2/libgcc.a).

## Supports the following libraries

	ctype.h
	debug.c
	debug.h (Use the GNU GCC MACROS __FILE__, __LINE__ and __func__)
	inttypes.h
	limits.h
	inttypes.h
	limits.h
	packed.h
	random.c
	random.h
	round.h
	stdarg.h
	stdbool.h
	stdio.h
	stddef.h
	stdint.h
	stdlib.c
	stdlib.h
	string.c
	string.h
	syscall-nr.h
	lib/kernel/bitmap.h
	lib/kernel/bitmap.c
	lib/kernel/list.h
	lib/kernel/list.c

# Directory structure

	src
		arm_asm
		devices
		lib
		lib/kernel
		libgcc
		threads
		config.txt
		kernel.ld
		Makefile
		
`arm_asm` contains the assembly files that are used to configure the Frame buffer, draw a pixel in the screen, control the GPIO, configure
the interruption handler. In addition to this, it contains the file `start.s`, which contains the function `_start`, which boots the system.
This is the first instruction from the Operating System that is executed.

`devices` contains the C files that control the timer, video, framebuffer and gpio.

`lib` contains util C files to handle strings, random numbers, define the size of the data types, etc.

`lib/kernel` contains C files that defined the basic data structures used by the kernel such as lists, hash tables and bitmaps.

`libgcc` contains the compile file  `libgcc.a` that implements the functions that are required to use the module and division operators.

`threads` contains the C files that implement the threads, synchronization, memory allocation and init.h. The first C function
that is executed is `init()`. This functions initializes the thread system, video system, memory systems and timers.

`config.txt` is a file that contains the screen configuration. It is read by the Raspberry PI firmware.

`kernel.ld` is the file that defines the layout of the memory, for example, where the compile code is going to live.

`Makefile` is the file that builds the kernel and generates a `kernel.img` that contains the kernel code.

# How to build the kernel

In the directory `src`, execute the following command:

	make

This command is going to generate the `src/kernel.img`. Copy this file into the SD card that will be used by the Raspberry PI.

The SD card must contain the following files at root level:

	bootcode.bin
	config.txt
	kernel.imag
	start.elf
	
The files `config.txt`, `bootcode.bin` and `start.elf` are located in the directory `backup-boot`.

Once that all these files are in the SD card, place the SD card in the Raspberry PI and connect it to the power. The HDMI
port should be connected to a monitor. In the monitor you should be able to see the threads in execution. 

# References

`ARM System Developer's Guide: Designing and Optimizing System Software 1st Edition`
by Andrew Sloss (Author), Dominic Symes (Author), Chris Wright (Author)

[Pintos](https://en.wikipedia.org/wiki/Pintos)

[Baking Pi](https://www.cl.cam.ac.uk/projects/raspberrypi/tutorials/os/index.html)






