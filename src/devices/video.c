
#include <stdbool.h>

#include "video.h"
#include "framebuffer.h"
#include "screen.h"
#include "../threads/interrupt.h"

#define BYTE 8  // Byte is equal to 8 bits.

// There are 4 types of copies (0-3), where the 0 is the fastest, and the 3 is the slowest.
#define VIDEO_MEMORY_COPY_METHOD 0

// Defines the maximum size that takes to print an int number (32 characters for binary base).
#define MAX_SIZE_INT_PRINT_NUMBER 32

/* Screen information. */
static struct screen_info screen;

/* Pointer to the frame buffer. */
static struct framebuffer_info *framebuffer;

/* DrawCharacter renders the image for a single character given in r0 to the
* screen, with to left corner given by (r1,r2). It is defined in drawing.s. */
extern void DrawCharacter(char character, uint32_t x, uint32_t y);

/*
 * DrawPixel draws a single pixel to the screen at the point in (r0,r1).
 * C++ Signature: void DrawPixel(u32 x u32 y);
 */
extern void DrawPixel(uint32_t x, uint32_t y);

/* Different implementations of memory copies found in memoryCopy.s. */
extern void memory_fastest_copy(char *src, char *dest, int size);
extern int memory_fast_copy(char *src, char *dest, int length);

/* Cleans the character area that starts in the position (x, y). */
static void video_clean_character(int x, int y);

/* Calculates the new positions of the coords X and Y. */
static void video_calculate_new_position();

/* Moves the current position (x, y) to the next row. If there are not more rows, it starts
 * at the beginning of the screen. */
static void video_new_line();

/* Cleans the characters from the given row. */
static void video_clean_row(int y);

/* Functions to display a new line in the screen. If the row is already
 * the last line on the screen, scrolls the screen upward one line.*/
static void video_new_line_fastest(void);
static void video_new_line_faster(void);
static void video_new_line_slower(void);
static void video_new_line_slowest(void);

void video_init() {
  screen.height = SCREEN_RESOLUTION_HEIGHT;
  screen.width = SCREEN_RESOLUTION_WIDTH;
  screen.pixel_size = SCREEN_PIXEL_SIZE;       // Size in bits
  screen.font_width = SCREEN_FONT_WIDTH;
  screen.font_height = SCREEN_FONT_HEIGHT;
  // Start writing at the top left corner of the screen.
  screen.x_position = 0;
  screen.y_position = 0;

  // Getting the framebuffer.
  framebuffer = (struct framebuffer_info *) GetGraphicsAddress();
}

/** Cleans the screen. */
void video_clean() {
  int black = 0;
  SetForeColour(black);
  int row;
  int col;
  for (row = 0; row < screen.height; row++) {
      for (col = 0; col < screen.width; col++) {
          DrawPixel(col, row);
      }
  }

  // Setting the (x, y) to zero.
  screen.x_position = 0;
  screen.y_position = 0;
}

/* Prints the given character in the screen. */
void video_putc(char character) {
  /* Disable interrupts to lock out interrupt handlers
     that might write to the console. */
  enum interrupts_level old_level = interrupts_disable();

  if (character == '\n') {
    video_new_line();
    //video_clean_row(screen.y_position);
  } else {
    // Clean the font area due a previous letter might be there.
    // TODO Review if we still need to call this method.
    video_clean_character(screen.x_position, screen.y_position);
    // DrawCharacter is define in drawing.s
    DrawCharacter(character, screen.x_position, screen.y_position);
    video_calculate_new_position();
  }

  interrupts_set_level(old_level);
}

/* Cleans the characters from the given row. */
static void video_clean_row(int y) {
  int row;
  int col;
  int foreColour = GetForeColour();
  SetForeColour(0);
  for (row = 0; row < screen.font_height; row++) {
      // Cleaning the whole row.
      for (col = 0; col < screen.width; col++) {
          DrawPixel(col, y + row);
      }
  }
  SetForeColour(foreColour);
}

/**
 * Cleans the character that is in the given position. After cleaning, there is a blank space
 * in the character area.
 */
static void video_clean_character(int x, int y) {
  int row;
  int col;
  int foreColour = GetForeColour();
  SetForeColour(0);
  for (row = 0; row < screen.font_height; row++) {
      for (col = 0; col < screen.font_width; col++) {
          DrawPixel(x + col, y + row);
      }
  }
  SetForeColour(foreColour);
}

/* Moves the current position (x, y) to the next row. If the row is already
 * the last line on the screen, scrolls the screen upward one line.
 */
static void video_new_line(void) {
  switch(VIDEO_MEMORY_COPY_METHOD) {
    case 0:
      video_new_line_fastest();
      break;
    case 1:
      video_new_line_faster();
      break;
    case 2:
      video_new_line_slower();
      break;
    case 3:
      video_new_line_slowest();
      break;
    default:
      video_new_line_fastest();
  }
}

/* Moves the current position (x, y) to the next row. If the row is already
 * the last line on the screen, scrolls the screen upward one line.
 *
 * Copies 10 consecutive words at a time. If the number of bytes to copy is not multiple of 40,
 * it copies 1 word at a time. If the remaining of this is copied byte by byte.
 *
 * Performance when it is necessary to scrolls the screen upward one line.:
 *
 *      BYTES_PER_LINE = WIDTH * BYTES PER PIXEL * FONT HEIGHT
 *      NUMBER_OF_BYTES_TO_MOVE = ((WIDTH * HEIGHT * BYTES PER PIXEL) - BYTES_PER_LINE)
 *
 * Example: WIDTH = 800, HEIGHT = 480, BYTES PER PIXEL = 2
 *      BYTES_PER_LINE = 800 * 2 * 16 = 25,600
 *      NUMBER_OF_BYTES_TO_MOVE = (800 * 480 * 2) - 25,600 = 76,8000 - 25,600 = 742,400
 *      ITERATIONS = 742,400 / 40 bytes copied at a time = 18,560
 *
 *      ---------------------------------------------------
 *      PERFORMANCE - 18,560 iterations
 *      ---------------------------------------------------
 */
static void video_new_line_fastest(void) {
  // Increasing the row.
  int new_y = screen.y_position + screen.font_height;

  if ((new_y + screen.font_height) > screen.height) {
    // Scrolls the screen upward one line.
    char *dest = (char *) framebuffer->gpu_pointer;

    int row_size = (screen.width * (screen.pixel_size / BYTE) * screen.font_height);
    char *src = dest + row_size;
    char *end_buffer =  dest + (screen.width * (screen.pixel_size / BYTE) * screen.height);

    int total = (end_buffer - src);
    // void memory_fastest_copy(src, dest, total) is defined in memoryCopy.s.
    memory_fastest_copy(src, dest, total);

    // Clean the last row.
    new_y = screen.y_position;
    video_clean_row(new_y);
  }

  screen.x_position = 0;
  screen.y_position = new_y;
}

/* Moves the current position (x, y) to the next row. If the row is already
 * the last line on the screen, scrolls the screen upward one line.
 *
 * Copies 10 consecutive words at a time. If the number of bytes to copy is not multiple of 40,
 * it returns the remaining bytes and they are copied byte by byte.
 *
 * Performance when it is necessary to scrolls the screen upward one line.:
 *
 *      BYTES_PER_LINE = WIDTH * BYTES PER PIXEL * FONT HEIGHT
 *      NUMBER_OF_BYTES_TO_MOVE = ((WIDTH * HEIGHT * BYTES PER PIXEL) - BYTES_PER_LINE)
 *
 * Example: WIDTH = 800, HEIGHT = 480, BYTES PER PIXEL = 2
 *      BYTES_PER_LINE = 800 * 2 * 16 = 25,600
 *      NUMBER_OF_BYTES_TO_MOVE = (800 * 480 * 2) - 25,600 = 76,8000 - 25,600 = 742,400
 *      ITERATIONS = 742,400 / 40 bytes copied at a time = 18,560
 *
 *      ---------------------------------------------------
 *      PERFORMANCE - 18,560 iterations
 *      ---------------------------------------------------
 */
static void video_new_line_faster(void) {
  // Increasing the row.
  int new_y = screen.y_position + screen.font_height;

  if ((new_y + screen.font_height) > screen.height) {
    // Scrolls the screen upward one line.
    char *dest = (char *) framebuffer->gpu_pointer;

    int row_size = (screen.width * (screen.pixel_size / BYTE) * screen.font_height);
    char *src = dest + row_size;
    char *end_buffer =  dest + (screen.width * (screen.pixel_size / BYTE) * screen.height);

    int total = (end_buffer - src);
    // int memory_fast_copy(src, dest, total) is defined in memoryCopy.s.
    int remaining = memory_fast_copy(src, dest, total);
    src = src + (total - remaining);
    dest = dest + (total - remaining);

    // Copy the remaining bytes.
    while(remaining-- > 0) {
        *dest++ = *src++;
    }

    // Clean the last row.
    new_y = screen.y_position;
    video_clean_row(new_y);
  }

  screen.x_position = 0;
  screen.y_position = new_y;
}

/* Moves the current position (x, y) to the next row. If the row is already
 * the last line on the screen, scrolls the screen upward one line.
 *
 * Copies 4 consecutive bytes at a time. It expects that the number of bytes to copy is a multiple
 * of 4.
 *
 * Performance when it is necessary to scrolls the screen upward one line.:
 *
 *      BYTES_PER_LINE = WIDTH * BYTES PER PIXEL * FONT HEIGHT
 *      NUMBER_OF_BYTES_TO_MOVE = ((WIDTH * HEIGHT * BYTES PER PIXEL) - BYTES_PER_LINE)
 *
 * Example: WIDTH = 800, HEIGHT = 480, BYTES PER PIXEL = 2
 *      BYTES_PER_LINE = 800 * 2 * 16 = 25,600
 *      NUMBER_OF_BYTES_TO_MOVE = (800 * 480 * 2) - 25,600 = 76,8000 - 25,600 = 742,400
 *      ITERATIONS = 742,400 / 4 bytes copied at a time = 185,600
 *
 *      ---------------------------------------------------
 *      PERFORMANCE - 185,600 iterations
 *      ---------------------------------------------------
 */
static void video_new_line_slower(void) {
  // Increasing the row.
  int new_y = screen.y_position + screen.font_height;

  if ((new_y + screen.font_height) > screen.height) {
    // Scrolls the screen upward one line.
    int *dest = (int *) framebuffer->gpu_pointer;

    int row_size = (screen.width * (screen.pixel_size / BYTE) * screen.font_height);
    int *src = (int *) ((char *) dest + row_size);
    char *end_buffer =  ((char *) dest + (screen.width * (screen.pixel_size / BYTE)  * screen.height));

    // The module of this division has to be ZERO.
    int total = (end_buffer - (char *) src) / 4; /* 4 bytes (Pointer Arithmetic) */
    while(total-- > 0) {
        /* int pointer arithmetic. */
        *dest++ = *src++; // Copies word by word
    }

    // Clean the last row.
    new_y = screen.y_position;
    video_clean_row(new_y);
  }

  screen.x_position = 0;
  screen.y_position = new_y;
}

/* Moves the current position (x, y) to the next row. If the row is already
 * the last line on the screen, scrolls the screen upward one line.
 *
 * Copies a byte at a time.
 *
 * Performance when it is necessary to scrolls the screen upward one line.:
 *
 *      BYTES_PER_LINE = WIDTH * BYTES PER PIXEL * FONT HEIGHT
 *      NUMBER_OF_BYTES_TO_MOVE = ((WIDTH * HEIGHT * BYTES PER PIXEL) - BYTES_PER_LINE)
 *
 * Example: WIDTH = 800, HEIGHT = 480, BYTES PER PIXEL = 2
 *      BYTES_PER_LINE = 800 * 2 * 16 = 25,600
 *      NUMBER_OF_BYTES_TO_MOVE = (800 * 480 * 2) - 25,600 = 76,8000 - 25,600 = 742,400
 *
 *      ---------------------------------------------------
 *      PERFORMANCE - 742,400 iterations
 *      ---------------------------------------------------
 */
static void video_new_line_slowest(void) {
  // Increasing the row.
  int new_y = screen.y_position + screen.font_height;

  if ((new_y + screen.font_height) > screen.height) {
    // Scrolls the screen upward one line.
    char *dest = (char *) framebuffer->gpu_pointer;

    int row_size = (screen.width * (screen.pixel_size / BYTE) * screen.font_height);
    char *src = dest + row_size;
    char *end_buffer =  dest + (screen.width * (screen.pixel_size / BYTE) * screen.height);

    int total = (end_buffer - src);
    while(total-- > 0) {
        *dest++ = *src++;       // Copies byte by byte.
    }

    // Clean the last row.
    new_y = screen.y_position;
    video_clean_row(new_y);
  }

  screen.x_position = 0;
  screen.y_position = new_y;
}

/**
 * Increments the values of x and y accordingly. It makes sure that the next character is drawn
 * in the appropriate screen area.
 */
static void video_calculate_new_position() {
  int new_x = screen.x_position + screen.font_width;
  int new_y = screen.y_position;

  // Verifying if the new character fix in the screen
  if ((new_x + screen.font_width) > screen.width) {
      video_new_line();
  } else {
      screen.x_position = new_x;
      screen.y_position = new_y;
  }
}



