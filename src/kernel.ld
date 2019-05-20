/******************************************************************************
*	kernel.ld
*
*	A linker script for generation of raspberry pi kernel images.
******************************************************************************/

SECTIONS {

	/*
	 * First and formost we need the .init section, containing the code to 
     * be run first. We allow room for the stack and conform to 
     * the bootloader's expectation by putting this code at 0x8000.
	 */
	.init 0x8000 : {
		*(.init)
	}
	
	/* 
     * Next we put the rest of the code.
	 */
	.text : {
		*(.text)
		*(.rodata)
	}
	
	/* 
	 * Next we put the data.
	 */
	.data : {
		*(.data)
	}

	/*
     * Unitialized data.
	 */
	.bss : {
		*(.bss)
	}

	/* Due we are using libgcc.a to link our project, it is expected that some variables and
	 * functions are provided due the linker is expecting them. And since we have an extreme bare
	 * metal system with custom memcpy(), no abort() etc, the libgcc does not link.
	 */
  	.ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) }
   	__exidx_start = .;
  	.ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) }
   	__exidx_end = .;
 
	/*
	 * Finally comes everything else. A fun trick here is to put all other 
	 * sections into this section, which will be discarded by default.
	 */
	/DISCARD/ : {
		*(*)
	}
}
