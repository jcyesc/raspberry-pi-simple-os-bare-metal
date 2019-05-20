#include "bitmap.h"

#include <debug.h>
#include <limits.h>
#include <round.h>
#include <stdio.h>

#include "../../threads/malloc.h"

/* Element type.

   This must be an unsigned integer type at least as wide as int.

   Each bit represents one bit in the bitmap.
   If bit 0 in an element represents bit K in the bitmap,
   then bit 1 in the element represents bit K+1 in the bitmap,
   and so on. */
typedef unsigned long elem_type;

/* Number of bits in an element. */
#define ELEM_BITS (sizeof (elem_type) * CHAR_BIT)

/* From the outside, a bitmap is an array of bits.  From the
   inside, it's an array of elem_type (defined above) that
   simulates an array of bits. */
struct bitmap {
    size_t bit_cnt;     /* Number of bits. */
    elem_type *bits;    /* Elements that represent bits. */
};

/* Returns the index of the element that contains the bit
   numbered BIT_IDX. */
static inline size_t elem_idx (size_t bit_idx) {
  return bit_idx / ELEM_BITS;
}

/* Returns an elem_type where only the bit corresponding to
   BIT_IDX is turned on. */
static inline elem_type bit_mask (size_t bit_idx) {
  return (elem_type) 1 << (bit_idx % ELEM_BITS);
}

/* Returns the number of elements required for BIT_CNT bits. */
static inline size_t elem_cnt (size_t bit_cnt) {
  return DIV_ROUND_UP (bit_cnt, ELEM_BITS);
}

/* Returns the number of bytes required for BIT_CNT bits. */
static inline size_t byte_cnt (size_t bit_cnt) {
  return sizeof (elem_type) * elem_cnt (bit_cnt);
}

/* Creation and destruction. */

/* Initializes B to be a bitmap of BIT_CNT bits and sets all of its bits to false.
   Returns true if success, false if memory allocation failed. */
struct bitmap *bitmap_create (size_t bit_cnt) {
  struct bitmap *b = malloc (sizeof *b);
  if (b != NULL) {
      b->bit_cnt = bit_cnt;
      b->bits = malloc (byte_cnt (bit_cnt));
      if (b->bits != NULL || bit_cnt == 0) {
          bitmap_set_all (b, false);
          return b;
      }
      free (b);
  }
  return NULL;
}

/* Creates and returns a bitmap with BIT_CNT bits in the
   BLOCK_SIZE bytes of storage preallocated at BLOCK.
   BLOCK_SIZE must be at least bitmap_needed_bytes(BIT_CNT). */
struct bitmap *bitmap_create_in_buf (size_t bit_cnt, void *block, size_t block_size UNUSED) {
  struct bitmap *b = block;

  ASSERT (block_size >= bitmap_buf_size (bit_cnt));

  b->bit_cnt = bit_cnt;
  b->bits = (elem_type *) (b + 1);
  bitmap_set_all (b, false);
  return b;
}

/* Returns the number of bytes required to accomodate a bitmap
   with BIT_CNT bits (for use with bitmap_create_in_buf()). */
size_t bitmap_buf_size (size_t bit_cnt) {
  return sizeof (struct bitmap) + byte_cnt (bit_cnt);
}

/* Destroys bitmap B, freeing its storage. Not for use on bitmaps created by
   bitmap_create_preallocated(). */
void bitmap_destroy (struct bitmap *b) {
  if (b != NULL) {
	  free (b->bits);
      free (b);
  }
}

/* Bitmap size. */

/* Returns the number of bits in B. */
size_t bitmap_size (const struct bitmap *b) {
  return b->bit_cnt;
}

/* Setting and testing single bits. */

/* Atomically sets the bit numbered IDX in B to VALUE. */
void bitmap_set (struct bitmap *b, size_t idx, bool value)  {
  ASSERT (b != NULL);
  ASSERT (idx < b->bit_cnt);
  if (value)
    bitmap_mark (b, idx);
  else
    bitmap_reset (b, idx);
}

/* Returns the value of the bit numbered IDX in B. */
bool bitmap_test (const struct bitmap *b, size_t idx) {
  ASSERT (b != NULL);
  ASSERT (idx < b->bit_cnt);
  return (b->bits[elem_idx (idx)] & bit_mask (idx)) != 0;
}

/* Setting and testing multiple bits. */

/* Sets all bits in B to VALUE. */
void bitmap_set_all (struct bitmap *b, bool value) {
  ASSERT (b != NULL);

  bitmap_set_multiple (b, 0, bitmap_size (b), value);
}

/* Sets the CNT bits starting at START in B to VALUE. */
void bitmap_set_multiple (struct bitmap *b, size_t start, size_t cnt, bool value) {
  size_t i;

  ASSERT (b != NULL);
  ASSERT (start <= b->bit_cnt);
  ASSERT (start + cnt <= b->bit_cnt);

  for (i = 0; i < cnt; i++)
    bitmap_set (b, start + i, value);
}

/* Atomically sets the bit numbered BIT_IDX in B to true. */
void bitmap_mark (struct bitmap *b, size_t bit_idx) {
  size_t idx = elem_idx (bit_idx);
  elem_type mask = bit_mask (bit_idx);
  b->bits[idx] |= mask;
}

/* Atomically sets the bit numbered BIT_IDX in B to false. */
void bitmap_reset (struct bitmap *b, size_t bit_idx) {
  size_t idx = elem_idx (bit_idx);
  elem_type mask = bit_mask (bit_idx);
  b->bits[idx] &= ~mask;
}

/* Returns true if any bits in B between START and START + CNT, exclusive, are set to VALUE,
   and false otherwise. */
bool bitmap_contains (const struct bitmap *b, size_t start, size_t cnt, bool value)  {
  size_t i;

  ASSERT (b != NULL);
  ASSERT (start <= b->bit_cnt);
  ASSERT (start + cnt <= b->bit_cnt);

  for (i = 0; i < cnt; i++)
    if (bitmap_test (b, start + i) == value)
      return true;
  return false;
}

/* Returns true if any bits in B between START and START + CNT, exclusive, are set to true, and
   false otherwise.*/
bool bitmap_any (const struct bitmap *b, size_t start, size_t cnt) {
  return bitmap_contains (b, start, cnt, true);
}

/* Returns true if no bits in B between START and START + CNT, exclusive, are set to true, and
   false otherwise.*/
bool bitmap_none (const struct bitmap *b, size_t start, size_t cnt)  {
  return !bitmap_contains (b, start, cnt, true);
}

/* Returns true if every bit in B between START and START + CNT,
   exclusive, is set to true, and false otherwise. */
bool bitmap_all (const struct bitmap *b, size_t start, size_t cnt) {
  return !bitmap_contains (b, start, cnt, false);
}

/* Finding set or unset bits. */

/* Finds and returns the starting index of the first group of CNT consecutive bits in B at or
   after START that are all set to VALUE. If there is no such group, returns BITMAP_ERROR. */
size_t bitmap_scan (const struct bitmap *b, size_t start, size_t cnt, bool value) {
  ASSERT (b != NULL);
  ASSERT (start <= b->bit_cnt);

  if (cnt <= b->bit_cnt) {
      size_t last = b->bit_cnt - cnt;
      size_t i;
      for (i = start; i <= last; i++) {
          if (!bitmap_contains (b, i, cnt, !value)) {
              return i;
          }
      }
  }
  return BITMAP_ERROR;
}

/* Finds the first group of CNT consecutive bits in B at or after START that are all set to VALUE,
   flips them all to !VALUE, and returns the index of the first bit in the group.
   If there is no such group, returns BITMAP_ERROR.
   If CNT is zero, returns 0.
   Bits are set atomically, but testing bits is not atomic with setting them. */
size_t bitmap_scan_and_flip (struct bitmap *b, size_t start, size_t cnt, bool value) {
  size_t idx = bitmap_scan (b, start, cnt, value);
  if (idx != BITMAP_ERROR)
    bitmap_set_multiple (b, idx, cnt, !value);
  return idx;
}

/* Debugging. */

/* Dumps the contents of B to the console as hexadecimal. */
void bitmap_dump (const struct bitmap *b)  {
  hex_dump (0, b->bits, byte_cnt (b->bit_cnt), false);
}






