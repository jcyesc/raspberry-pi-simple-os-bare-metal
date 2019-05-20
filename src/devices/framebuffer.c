/*
 * framebuffer.c contains the code that initializes the framebuffer.
 */

#include <stdbool.h>
#include <stddef.h>

#include "framebuffer.h"
#include "gpio.h"
#include "screen.h"

/* Function defined in frameBufferInfo.s. */
extern struct framebuffer_info *
InitialiseFrameBuffer(unsigned int width, unsigned int height, unsigned int bit_depth);

/* Function defined in drawing.s. */
extern void SetGraphicsAddress(struct framebuffer_info * framebuffer);

/* Initializes  the frame buffer. It sets the address of the framebuffer_info that contains
 * information about the frame buffer. This procedure blocks until a frame buffer can be created,
 * and so is inappropriate on real time systems. If the frame buffer cannot be created, this
 * procedure HANGS forever and the ACT led is TURN ON. */
void framebuffer_init() {
  struct framebuffer_info *framebuffer = InitialiseFrameBuffer(SCREEN_RESOLUTION_WIDTH,
      SCREEN_RESOLUTION_HEIGHT,
      SCREEN_PIXEL_SIZE);

  if (framebuffer == NULL) {
      gpio_enable_function(16, 1);
      gpio_set_register(16, 0);
      /* Note: PANIC(...) can't be used here because the frame buffer is used to draw characters
       * on the screen. */
      while(true);      /* Hang forever due there is an error. */
  }

  SetGraphicsAddress(framebuffer); /* Sets the graphic address. */
}
