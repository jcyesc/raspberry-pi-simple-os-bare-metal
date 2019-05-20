##############################################################################################
#	makefile
#
#	A makefile script for generation of BearOs raspberry pi kernel images.
##############################################################################################

# The toolchain to use. arm-none-eabi works, but there does exist 
# arm-bcm2708-linux-gnueabi.
ARMGNU = arm-none-eabi

# The intermediate directory for compiled object files.
BUILD = build/

# The directory that contains the device C files.
DEVICES = devices/

# The directory that contains the miscellanea C files.
MISC = misc/

# The directory that contains the threads C files.
THREADS = threads/

# The directory that contains the C libraries.
LIB = lib/

# The directory that contains the C kernel libraries.
LIB_KERNEL = lib/kernel/

##############################################################################################
# GCC library
# GCC provides a low-level runtime library, libgcc.a or libgcc_s.so.1 on some platforms.
# GCC generates calls to routines in this library automatically, whenever it needs to perform some
# operation that is too complicated to emit inline code for.
#
# Most of the routines in libgcc handle arithmetic operations that the target processor cannot
# perform directly. This includes integer multiply and divide on some machines, and all
# floating-point and fixed-point operations on other machines. libgcc also includes routines for
# exception handling, and a handful of miscellaneous operations. 
#
# https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html
#
# https://gcc.gnu.org/onlinedocs/gccint/Integer-library-routines.html#Integer-library-routines
#
##############################################################################################
LIB_GCC = libgcc/libgcc.a

# The directory in which source files are stored.
ASM_SOURCE = arm_asm/

# SD card
SD_CARD = /Volumes/RECOVERY

# The name of the output file to generate.
TARGET = kernel.img

# The name of the assembler listing file to generate.
LIST = kernel.list

# The name of the map file to generate.
MAP = kernel.map

# The name of the linker script to use.
LINKER = kernel.ld

# C FLAGS
# -nostdinc		No include the standard libraries.
# -I$(LIB)		Include the standard libraries.
CFLAGS = -nostdinc -I$(LIB) -I$(LIB_KERNEL)
CFLAGS += -mcpu=arm1176jzf-s
CFLAGS += -Wall

# The names of all object files that must be generated. Deduced from the 
# assembly code files in source.
OBJECTS := $(patsubst $(ASM_SOURCE)%.s,$(BUILD)%.o,$(wildcard $(ASM_SOURCE)*.s))

# Rule to make everything.
all: $(TARGET) $(LIST)

# Rule to remake everything. Does not include clean.
rebuild: all

# Rule to copy the image onto the flash drive.
install : rebuild
	cp $(TARGET) $(SD_CARD)
	#umount $(SD_CARD)

# Rule to make the listing file.
$(LIST) : $(BUILD)output.elf
	$(ARMGNU)-objdump -d $(BUILD)output.elf > $(LIST)

# Rule to make the image file.
$(TARGET) : $(BUILD)output.elf
	$(ARMGNU)-objcopy $(BUILD)output.elf -O binary $(TARGET) 

# C Objects that have to be compiled.
C_OBJECTS = $(BUILD)bitmap.o
C_OBJECTS += $(BUILD)console.o
C_OBJECTS += $(BUILD)debug.o
C_OBJECTS += $(BUILD)framebuffer.o
C_OBJECTS += $(BUILD)gpio.o
C_OBJECTS += $(BUILD)hash.o
C_OBJECTS += $(BUILD)init.o
C_OBJECTS += $(BUILD)list.o
C_OBJECTS += $(BUILD)interrupt.o
C_OBJECTS += $(BUILD)malloc.o
C_OBJECTS += $(BUILD)palloc.o
C_OBJECTS += $(BUILD)random.o
C_OBJECTS += $(BUILD)serial.o
C_OBJECTS += $(BUILD)stdio.o
C_OBJECTS += $(BUILD)stdlib.o
C_OBJECTS += $(BUILD)string.o
C_OBJECTS += $(BUILD)synch.o
C_OBJECTS += $(BUILD)timer.o
C_OBJECTS += $(BUILD)thread.o
C_OBJECTS += $(BUILD)video.o

# Rule to make the elf file.
$(BUILD)output.elf : $(OBJECTS) $(C_OBJECTS) $(LINKER)
	$(ARMGNU)-ld --no-undefined $(OBJECTS) $(C_OBJECTS) \
	-Map $(MAP) -o $(BUILD)output.elf -T $(LINKER) \
	 $(LIB_GCC)
# 	-verbose 

# Rule to make the object files.
$(BUILD)%.o: $(ASM_SOURCE)%.s $(BUILD)
	$(ARMGNU)-as -I $(ASM_SOURCE) $< -o $@

# Rule to make the list object files.
$(BUILD)bitmap.o: $(LIB_KERNEL)bitmap.h $(LIB_KERNEL)bitmap.c $(THREADS)malloc.h $(BUILD)
	$(ARMGNU)-gcc $(CFLAGS) -c $(LIB_KERNEL)bitmap.c -o $(BUILD)bitmap.o

# Rule to make the console object files
$(BUILD)console.o: $(LIB_KERNEL)console.h $(DEVICES)framebuffer.h $(DEVICES)screen.h $(LIB)stdbool.h $(LIB_KERNEL)console.c $(THREADS)interrupt.h $(BUILD)
	$(ARMGNU)-gcc $(CFLAGS) -c $(LIB_KERNEL)console.c -o $(BUILD)console.o

# Rule to make the timer object files.
$(BUILD)debug.o: $(LIB)debug.h $(LIB)debug.c $(BUILD)
	$(ARMGNU)-gcc $(CFLAGS) -c $(LIB)debug.c -o $(BUILD)debug.o

# Rule to make the framebuffer object files.
$(BUILD)framebuffer.o: $(DEVICES)gpio.h $(DEVICES)framebuffer.h $(DEVICES)screen.h $(DEVICES)framebuffer.c $(BUILD)
	$(ARMGNU)-gcc $(CFLAGS) -c $(DEVICES)framebuffer.c -o $(BUILD)framebuffer.o

# Rule to make the framebuffer object files.
$(BUILD)gpio.o: $(DEVICES)gpio.h $(DEVICES)gpio.c $(BUILD)
	$(ARMGNU)-gcc $(CFLAGS) -c $(DEVICES)gpio.c -o $(BUILD)gpio.o

# Rule to make the hash object files.
$(BUILD)hash.o: $(LIB_KERNEL)hash.h $(LIB_KERNEL)hash.c $(BUILD)
	$(ARMGNU)-gcc $(CFLAGS) -c $(LIB_KERNEL)hash.c -o $(BUILD)hash.o
	
# Rule to make the init object files.
$(BUILD)init.o: $(THREADS)init.h $(THREADS)malloc.h $(THREADS)synch.h $(THREADS)interrupt.h $(THREADS)init.c $(BUILD)
	$(ARMGNU)-gcc $(CFLAGS) -c $(THREADS)init.c -o $(BUILD)init.o

# Rule to make the interrupt object files.
$(BUILD)interrupt.o: $(THREADS)interrupt.h $(THREADS)flags.h  $(LIB)stdbool.h $(DEVICES)bcm2835.h $(THREADS)interrupt.c $(BUILD)
	$(ARMGNU)-gcc $(CFLAGS) -c $(THREADS)interrupt.c -o $(BUILD)interrupt.o

# Rule to make the list object files.
$(BUILD)list.o: $(LIB_KERNEL)list.h $(LIB_KERNEL)list.c $(BUILD)
	$(ARMGNU)-gcc $(CFLAGS) -c $(LIB_KERNEL)list.c -o $(BUILD)list.o

# Rule to make the palloc object files.
$(BUILD)malloc.o: $(THREADS)malloc.h $(THREADS)malloc.c $(BUILD)
	$(ARMGNU)-gcc $(CFLAGS) -c $(THREADS)malloc.c -o $(BUILD)malloc.o

# Rule to make the palloc object files.
$(BUILD)palloc.o: $(THREADS)palloc.h $(THREADS)palloc.c $(BUILD)
	$(ARMGNU)-gcc $(CFLAGS) -c $(THREADS)palloc.c -o $(BUILD)palloc.o

# Rule to make the random object files.
$(BUILD)random.o: $(LIB)random.h $(LIB)random.c $(BUILD)
	$(ARMGNU)-gcc $(CFLAGS) -c $(LIB)random.c -o $(BUILD)random.o

# Rule to make the serial object files.
$(BUILD)serial.o: $(DEVICES)serial.h $(DEVICES)serial.c $(BUILD)
	$(ARMGNU)-gcc $(CFLAGS) -c $(DEVICES)serial.c -o $(BUILD)serial.o

# Rule to make the stdio object files.
$(BUILD)stdio.o: $(LIB)stdio.h $(LIB)stdio.c $(BUILD)
	$(ARMGNU)-gcc $(CFLAGS) -c $(LIB)stdio.c -o $(BUILD)stdio.o

# Rule to make the stdlib object files.
$(BUILD)stdlib.o: $(LIB)stdlib.h $(LIB)stdlib.c $(BUILD)
	$(ARMGNU)-gcc $(CFLAGS) -c $(LIB)stdlib.c -o $(BUILD)stdlib.o

# Rule to make the string object files.
$(BUILD)string.o: $(LIB)string.h $(LIB)string.c $(BUILD)
	$(ARMGNU)-gcc $(CFLAGS) -c $(LIB)string.c -o $(BUILD)string.o

# Rule to make the sync object files.
$(BUILD)synch.o: $(THREADS)synch.h  $(THREADS)thread.h $(LIB_KERNEL)list.h $(THREADS)synch.c $(BUILD)
	$(ARMGNU)-gcc $(CFLAGS) -c $(THREADS)synch.c -o $(BUILD)synch.o

# Rule to make the timer object files.
$(BUILD)timer.o: $(DEVICES)bcm2835.h $(DEVICES)timer.h $(DEVICES)timer.c $(BUILD)
	$(ARMGNU)-gcc $(CFLAGS) -c $(DEVICES)timer.c -o $(BUILD)timer.o

# Rule to make the thread object files.
$(BUILD)thread.o: $(THREADS)interrupt.h $(THREADS)flags.h $(THREADS)vaddr.h $(THREADS)thread.h $(THREADS)thread.c $(BUILD)
	$(ARMGNU)-gcc $(CFLAGS) -c $(THREADS)thread.c -o $(BUILD)thread.o

# Rule to make the video object files.
$(BUILD)video.o: $(DEVICES)video.h $(DEVICES)video.c $(THREADS)interrupt.h $(BUILD)
	$(ARMGNU)-gcc $(CFLAGS) -c $(DEVICES)video.c -o $(BUILD)video.o

$(BUILD):
	mkdir $@

# Rule to clean files.
clean : 
	-rm -rf $(BUILD)
	-rm -f $(TARGET)
	-rm -f $(LIST)
	-rm -f $(MAP)
