#ifndef ELF_H
#define ELF_H

#include <stdinc.h>
#include <multiboot.h>

/* Used to get the symbol type */
#define ELF32_ST_TYPE(i) ((i)&0xf)

/* List of symbol types */
#define ELF32_TYPE_FUNCTION (0x02)

/* A section header with all kinds of useful information */
typedef struct
{
  uint32_t name_offset_in_shstrtab;
  uint32_t type;
  uint32_t flags;
  uint32_t addr;
  uint32_t offset;
  uint32_t size;
  uint32_t link;
  uint32_t info;
  uint32_t addralign;
  uint32_t entsize;
} __attribute__((packed)) elf_section_header_t;
  
/* A symbol itself */
typedef struct
{
  uint32_t name_offset_in_strtab;
  uint32_t value;
  uint32_t size;
  uint8_t  info;
  uint8_t  other;
  uint16_t shndx;
} __attribute__((packed)) elf_symbol_t;

/* Will hold the array of symbols and their names for us */
typedef struct
{
  elf_symbol_t *symtab;
  uint32_t      symtab_size;
  const char   *strtab;
  uint32_t      strtab_size;
} elf_symbols_t;

 
/* Builds as the set of elf symbols from a multiboot scructure */
void build_elf_symbols_from_multiboot (multiboot_t* mb);

/* Locate a symbol (only functions) in the following elf symbols
 * Note that, for now, we'll only use this for the kernel (no other elf things..)
 */
const char* elf_lookup_symbol_function (uint32_t addr, elf_symbols_t* elf);

/* Same function as above, but for the kernel */
const char* kernel_elf_lookup_symbol_function (uint32_t addr);
#endif
