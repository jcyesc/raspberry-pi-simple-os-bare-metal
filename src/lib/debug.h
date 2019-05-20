
#ifndef LIB_DEBUG_H_
#define LIB_DEBUG_H_

/* GCC lets us add "attributes" to functions, function
   parameters, etc. to indicate their properties.
   See the GCC manual for details. */
#define UNUSED __attribute__ ((unused))
#define NO_RETURN __attribute__ ((noreturn))
#define NO_INLINE __attribute__ ((noinline))
#define PRINTF_FORMAT(FMT, FIRST) __attribute__ ((format (printf, FMT, FIRST)))

/* Halts the OS, printing the message and the condition.
 * Macros define by gcc.
 *      __FILE__        File name
 *      __LINE__        Line number
 *      __func__        Function name
 *      __VA_ARGS__     Extra parameters
 */
#define PANIC(...) debug_panic(__FILE__, __LINE__, __func__, __VA_ARGS__)

#define ASSERT(CONDITION)                       \
        if (CONDITION) { } else {               \
                PANIC ("Assertion failed");     \
        }

#define NOT_REACHED() PANIC ("executed an unreachable statement");

/* Halts the OS, printing the source file name, line number, and
   function name, plus a user-specific message. */
void debug_panic (const char *file, int line, const char *function,
                  const char *message, ...) NO_RETURN;

/* Prints the bits of data. */
void debug_print_bits_int(int data);

/* It seems to TARGET=arm-none-eabi" introduces dependencies of libc functions like memcpy() and
 * abort() that did not exist in previous versions, so it is necessary to provide them.
 *
 * https://gcc.gnu.org/ml/gcc-help/2012-03/msg00364.html
 */
void abort();

#endif /* LIB_DEBUG_H_ */
