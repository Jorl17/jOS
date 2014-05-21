#include <elf.h>
#include <string.h>
#include <screen.h>

PRIVATE elf_symbols_t kernel_elf_symbols;

/*
 * This function grabs a pointer to the array of section headers.
 * It then grabs a pointer to the section where all the strings are (.shstrtab)
 * (remember that each section header has an offset to this .shstrtab)
 * 
 * Then it's just a matter of iterating sections (checking their names via indexing
 * shstrtab) until we find strtab and symtab, which is what we're looking for
 */
void build_elf_symbols_from_multiboot ( multiboot_t* mb )
{
    uint32_t i;
    elf_section_header_t* sh = ( elf_section_header_t* ) mb->shdr_addr;

    /* .shstrtab has the names of the sections,
     * and sh is an array of sections, which themselves contain
     * an index to .shstrtab (for their names)
     */
    uint32_t shstrtab = sh[mb->shndx].addr;
    for ( i = 0; i < mb->shdr_num; i++ ) {
        const char *name = ( const char* ) ( shstrtab + sh[i].name_offset_in_shstrtab );
        if ( !strcmp ( name, ".strtab" ) ) {
            kernel_elf_symbols.strtab = ( const char * ) sh[i].addr;
            kernel_elf_symbols.strtab_size = sh[i].size;
        }
        if ( !strcmp ( name, ".symtab" ) ) {
            kernel_elf_symbols.symtab = ( elf_symbol_t* ) sh[i].addr;
            kernel_elf_symbols.symtab_size = sh[i].size;
        }
    }
}

/*
 * Iterate through all the symbols and look for functions...
 * Then, as we find functions, check if the symbol is within that
 * function's range (given by value and size)
 */
const char* elf_lookup_symbol_function ( uint32_t addr, elf_symbols_t* elf )
{
    int i;
    int num_symbols = elf->symtab_size / sizeof ( elf_symbol_t );

    /*screen_puts("Num symbols: "); screen_put_int(num_symbols); screen_puts("\n");*/
    for ( i = 0; i < num_symbols; i++ ) {
        /*screen_puts("Processing a symbol..."); 
        screen_puts(elf->strtab + elf->symtab[i].name_offset_in_strtab);
        screen_puts("\n");         */
        if ( ELF32_ST_TYPE ( elf->symtab[i].info ) != ELF32_TYPE_FUNCTION )
            continue;

        if ( ( addr >= elf->symtab[i].value ) &&
                ( addr < ( elf->symtab[i].value + elf->symtab[i].size ) ) ) {
            const char* name = ( const char* ) ( ( uint32_t ) elf->strtab + elf->symtab[i].name_offset_in_strtab );
            return name;
        }
    }
    
    return NULL;
}

const char* kernel_elf_lookup_symbol_function ( uint32_t addr )
{
    return elf_lookup_symbol_function(addr, &kernel_elf_symbols);
}
