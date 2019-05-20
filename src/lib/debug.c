
#include <stdio.h>

#include "debug.h"
#include "../threads/interrupt.h"


/* Halts the OS, printing the source file name, line number, and
   function name, plus a user-specific message. */
void debug_panic (const char *file, int line, const char *function,
                  const char *message, ...) {

  interrupts_disable();

  unsigned short red = 0xF800;
  SetForeColour(red);
  printf("\nKernel PANIC at:");
  printf("\nFile: %s", file);
  printf("\nLine: %d", line);
  printf("\nFunction: %s", function);
  printf("\nMessage: %s", message);

  /* Halting forever. */
  for (;;);
}

/* Prints the bits of data. */
void debug_print_bits_int(int data) {
  int mask;
  int i;
  for (i = 31; i >= 0; i--) {
      mask = 1 << i;
      int value = data & mask;
      if (value != 0) {
          printf("1");
      } else {
          printf("0");
      }
  }
}

/* It seems to TARGET=arm-none-eabi" introduces dependencies of libc functions like memcpy() and
 * abort() that did not exist in previous versions, so it is necessary to provide them.
 *
 * https://gcc.gnu.org/ml/gcc-help/2012-03/msg00364.html
 */
void abort(void) {
  PANIC("Aborting....");
}
