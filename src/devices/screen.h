/*
 * monitor.h
 */

#ifndef SCREEN_LIB_H_
#define SCREEN_LIB_H_

/* High Color 65,536
 *
 * Use 16 bits to store each pixel, the first 5 bit representing the intensity of the red channel,
 * the next 6 bits representing the intensity of the green channel and the final 5 bits representing
 * the intensity of the blue channel.
 */

/* Screen width in pixels. */
#define SCREEN_RESOLUTION_WIDTH 800

/* Screen height in pixels. */
#define SCREEN_RESOLUTION_HEIGHT 480

/* Screen pixel size in bits (High color resolution) */
#define SCREEN_PIXEL_SIZE 16

/* Screen font height in pixels. */
#define SCREEN_FONT_HEIGHT 16

/* Screen font width in pixels. */
#define SCREEN_FONT_WIDTH 8

struct screen_info {
  int height;    /* Height in pixels. */
  int width;     /* Width in pixels. */
  int pixel_size; /* Pixel size in bits*/
  int font_width;  /* Font width in pixels. */
  int font_height;  /* Font height in pixels. */
  int x_position; /* X position where the next character will be painted. */
  int y_position; /* Y position where the next character will be painted. */
};


#endif /* SCREEN_LIB_H_ */
