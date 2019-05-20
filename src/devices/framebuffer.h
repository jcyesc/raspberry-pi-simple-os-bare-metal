
#ifndef DEVICES_FRAMEBUFFER_C_
#define DEVICES_FRAMEBUFFER_C_

/* When communicating with the graphics card about frame buffers, a message consists of a pointer
 * to the structure below. The comments explain what each memer of the structure is.
 *
 */
struct framebuffer_info {
  int width;            /* Width */
  int height;           /* Height */
  int virtual_width;    /* Virtual Width */
  int virtual_height;   /* Virtual Height */
  int gpu_pitch;        /* GPU - Pitch */
  int bit_depth;        /* Bit depth*/
  int x;                /* X */
  int y;                /* Y */
  void *gpu_pointer;    /* GPU - Pointer to the address where the screen content can be put. */
  int gpu_size;         /* GPU - Size of the memory. */
};

/* Initializes  the frame buffer. It sets the address of the framebuffer_info that contains
 * information about the frame buffer. This procedure blocks until a frame buffer can be created,
 * and so is inappropriate on real time systems. If the frame buffer cannot be created, this
 * procedure HANGS forever and the ACT led is TURN ON. */
void framebuffer_init();

#endif /* DEVICES_FRAMEBUFFER_C_ */
