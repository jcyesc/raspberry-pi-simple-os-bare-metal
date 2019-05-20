
#ifndef DEVICES_VGA_H_
#define DEVICES_VGA_H_

#include <stdint.h>

/* Initializes the video. */
void video_init();

/* Prints the given character in the console. */
void video_putc (char);

/* Cleans the console (whole screen). */
void video_clean();

/*
 * SetForeColour changes the current drawing colour to the 16 bit colour in r0.
 * C++ Signature: void SetForeColour(u16 colour);
 */
extern void SetForeColour(uint16_t colour);

/*
* GetForeColour returns the current drawing colour to the 16 bit colour in r0.
* C++ Signature: short GetForeColour();
*/
extern uint16_t GetForeColour();

#endif /* DEVICES_VGA_H_ */
