/******************************************************************************
*	drawing.s
*
*	drawing.s contains code to do with drawing shapes to the screen.
******************************************************************************/

/*
* The foreColour is the colour which all our methods will draw shapes in.
* C++ Signature: short foreColour;
*/
.section .data
.align 1
foreColour:
	.hword 0xFFFF

/*
* graphicsAddress stores the address of the frame buffer info structure. 
* C++ Signature: FrameBuferDescription* graphicsAddress;
*/
.align 2
graphicsAddress:
	.int 0

/* NEW
* Font stores the bitmap images for the first 128 characters.
*/
.align 4
font:
	.incbin "font.bin"

/*
* SetForeColour changes the current drawing colour to the 16 bit colour in r0.
* C++ Signature: void SetForeColour(u16 colour);
*/
.section .text
.globl SetForeColour
SetForeColour:
	cmp r0,#0x10000
	movhi pc,lr
	moveq pc,lr

	ldr r1,=foreColour
	strh r0,[r1]
	mov pc,lr

/*
* GetForeColour returns the current drawing colour to the 16 bit colour in r0.
* C++ Signature: short GetForeColour();
*/
.section .text
.globl GetForeColour
GetForeColour:
	ldr r0,=foreColour
	ldrh r0,[r0]
	mov pc,lr



/*
* SetGraphicsAddress changes the current frame buffer information to 
* graphicsAddress;
* C++ Signature: void SetGraphicsAddress(FrameBuferDescription* value);
*/
.globl SetGraphicsAddress
SetGraphicsAddress:
	ldr r1,=graphicsAddress
	str r0,[r1]
	mov pc,lr


/*
* GetGraphicsAddress returns the address of the graphicsAddress;
* C++ Signature: FrameBuferDescription* GetGraphicsAddress();
*/
.globl GetGraphicsAddress
GetGraphicsAddress:
	ldr r0,=graphicsAddress
	ldr r0,[r0]					// Deferencing the pointer.
	mov pc,lr

/*
* DrawPixel draws a single pixel to the screen at the point in (r0,r1).
* C++ Signature: void DrawPixel(u32 x u32 y);
*/
.globl DrawPixel
DrawPixel:
	px .req r0
	py .req r1
	
	addr .req r2
	ldr addr,=graphicsAddress
	ldr addr,[addr]
	
	height .req r3
	ldr height,[addr,#4]
	sub height,#1
	cmp py,height
	movhi pc,lr
	.unreq height
	
	width .req r3
	ldr width,[addr,#0]
	sub width,#1
	cmp px,width
	movhi pc,lr
	
	ldr addr,[addr,#32]
	add width,#1
	mla px,py,width,px
	.unreq width
	.unreq py
	add addr, px,lsl #1
	.unreq px

	fore .req r3
	ldr fore,=foreColour
	ldrh fore,[fore]
	
	strh fore,[addr]
	.unreq fore
	.unreq addr
	mov pc,lr

/*
* DrawLine draws a line between two points given in (r0,r1) and (r2,r3).
* Uses Bresenham's Line Algortihm
* C++ Signature: void DrawLine(u32x2 p1, u32x2 p2);
*/
.globl DrawLine
DrawLine:
	push {r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
	x0 .req r9
	x1 .req r10
	y0 .req r11
	y1 .req r12

	mov x0,r0
	mov x1,r2
	mov y0,r1
	mov y1,r3

	dx .req r4
	dyn .req r5 /* Note that we only ever use -deltay, so I store its negative for speed. (hence dyn) */
	sx .req r6
	sy .req r7
	err .req r8

	cmp x0,x1
	subgt dx,x0,x1
	movgt sx,#-1
	suble dx,x1,x0
	movle sx,#1
	
	cmp y0,y1
	subgt dyn,y1,y0
	movgt sy,#-1
	suble dyn,y0,y1
	movle sy,#1

	add err,dx,dyn
	add x1,sx
	add y1,sy

	pixelLoop$:
		teq x0,x1
		teqne y0,y1
		popeq {r4,r5,r6,r7,r8,r9,r10,r11,r12,pc}

		mov r0,x0
		mov r1,y0
		bl DrawPixel

		cmp dyn, err,lsl #1
		addle err,dyn
		addle x0,sx

		cmp dx, err,lsl #1
		addge err,dx
		addge y0,sy

		b pixelLoop$

	.unreq x0
	.unreq x1
	.unreq y0
	.unreq y1
	.unreq dx
	.unreq dyn
	.unreq sx
	.unreq sy
	.unreq err

/*
* DrawCharacter renders the image for a single character given in r0 to the
* screen, with to left corner given by (r1,r2), and returns the width of the 
* printed character in r0, and the height in r1.
* C++ Signature: u32x2 DrawCharacter(char character, u32 x, u32 y);
*/
.globl DrawCharacter
DrawCharacter:
	x .req r4
	y .req r5
	charAddr .req r6

	cmp r0,#0x7F
	movhi r0,#0
	movhi r1,#0
	movhi pc,lr

	mov x,r1
	mov y,r2

	push {r4,r5,r6,r7,r8,lr}
	ldr charAddr,=font
	add charAddr, r0,lsl #4
	
	lineLoop$:
		bits .req r7
		bit .req r8
		ldrb bits,[charAddr]		
		mov bit,#8

		charPixelLoop$:
			subs bit,#1
			blt charPixelLoopEnd$
			lsl bits,#1
			tst bits,#0x100
			beq charPixelLoop$

			add r0,x,bit
			mov r1,y
			bl DrawPixel

			b charPixelLoop$
		charPixelLoopEnd$:

		.unreq bit
		.unreq bits
		add y,#1
		add charAddr,#1
		tst charAddr,#0b1111
		bne lineLoop$

	.unreq x
	.unreq y
	.unreq charAddr

	width .req r0
	height .req r1
	mov width,#8
	mov height,#16

	pop {r4,r5,r6,r7,r8,pc}
	.unreq width
	.unreq height

