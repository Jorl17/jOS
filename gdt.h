#ifndef GDT_H
#define GDT_H
#include <stdinc.h>

void init_gdt ();

/*
 * In a segment system, each segment is described by a BASE address, of 32 bits,
 * and a 20-bit LIMIT address. The 12 bit difference is not a problem because
 * LIMIT need not be in bytes. In fact, the granularity field of a GDT entry
 * defines if LIMIT is in bytes or multiples of 4kbytes (page-size).
 * Thus, if we choose granularity such that LIMTI comes in 4kbytes, then we can
 * reach address 0xFFFFFFFF, thus 'cancelling out' the 12 bit difference in
 * some way
 *
 *
 * Today we mostly opt for a FLAT MEMORY MODEL, in which the base address is 0
 * and it goes all the way up to 0xFFFFFFFF for both the DATA segment and CODE
 * segment.
 */

typedef struct
{
  uint16_t limit_low;     /* The lower 16 bits of the limit. */
  uint16_t base_low;      /* The lower 16 bits of the base. */
  uint8_t base_middle;    /* The next 8 bits of the base. */
  uint8_t access;         /* Access flags, see below */
  uint8_t granularity;    /* Granularity, see below */
  uint8_t base_high;      /* The last 8 bits of the base. */
} __attribute__((packed)) gdt_entry_t;

/*
 * Format of Access flags:
   -------------------------------------------------------------------------------------------------------------
   | Present bit (must be 1) | Ring level (0-3) | Always 1  | Executable | Dir/confo Bit | RW bit | Access Bit |
   |          1 bit          |      2 bits      |  1 bit    |   1 bit    |     1 bit     | 1 bit  |    1 bit   |
   -------------------------------------------------------------------------------------------------------------
   
   Present: Must be 1
   Ring level: 0-3
   Executable: Marks if it is executable. 1 = CODE selector, 0 = DATA selector
   Direction / Conforming bit: 
        For DATA selectors, Tells the direction. 0 the segment grows up. 1 the
           segment grows down, ie. the offset has to be greater than the base.
        For CODE selectors, If 1 code in this segment can be executed from an
           equal or lower privilege level. For example, code in ring 3 can
           far-jump to conforming code in a ring 2 segment. The privl-bits
           represent the highest privilege level that is allowed to execute the
           segment. For example, code in ring 0 cannot far-jump to a conforming
           code segment with privl==0x2, while code in ring 2 and 3 can. Note
           that the privilege level remains the same, ie. a far-jump form ring 3
           to a privl==2-segment remains in ring 3 after the jump. If 0 code in
           this segment can only be executed from the ring set in privl.
   RW Bit:
        Readable bit for code selectors: Whether read access for this segment is
           allowed. Write access is never allowed for code segments.
        Writable bit for data selectors: Whether write access for this segment is
           allowed. Read access is always allowed for data segments.
  Access Bit: Just set to 0. The CPU sets this to 1 when the segment is accessed
*/

/*
 * Format of Granularity:
   -------------------------------------------------------------------------------------------------------------------------
   | G (0 = 1byte, 1=4kbytes( | D (operand size 0= 16 bit, 1 = 32 bit)) | Always 0  | Avail (should be 0) | Segment Length |
   |          1 bit           |                  1 bit                  |  1 bit    |        1 bit        |    4 bits      |
   -------------------------------------------------------------------------------------------------------------------------
*/

/*       Pr   R 1 E D/C RW A */
/* Code: 1  000 1 1 0   1  0 --> 0x9A*/
/* Data: 1  000 1 0 0   1  0 --> 0x92*/
#define CODE_SELECTOR 0x9A
#define DATA_SELECTOR 0x92

/*                   G D 0 A SegLen */
/* Flat-Model Priv:  1 1 0 1   1111 --> 0xCF*/
#define FLATMODEL_GRAN  0xCF
#define FLATMODEL_BASE  0x0
#define FLATMODEL_LIMIT 0xFFFFF



/* This struct describes a GDT pointer. It points to the start of
   our array of GDT entries, and is in the format required by the
   lgdt instruction. */
typedef struct
{
    uint16_t limit;       /* The Global Descriptor Table limit. (table size -1) */
    uint32_t base;        /* The address of the first gdt_entry_t struct. */
} __attribute__((packed)) gdt_ptr_t;

#endif
