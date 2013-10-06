#include "vmm.h"
#include "pmm.h"
#include <string.h>
#include <kpanic.h>
#include <screen.h>

/*
 * HOW THE VMM IS SETUP ON jOS
 * First, go read start.s! It documents how paging is when the kernel boots,
 * and you should _really_ go read it!
 * 
 * Second, the paging system in jOS is actually really simple for now, and part
 * of this has been said in start.s. The PMM can give us pages starting at
 * 0xB0000000 and going up to 0xC0000000 (though we aren't enforcing the
 * upper limit, it's good if we do, because the kernel code starts at
 * 0xC0000000, at least in terms of virtual addresses.
 * 
 * To ease things on us, 0xB0000000 is identity-mapped for the first 4MB
 * of memory, except that's done with blocks of 4KB pages and not with
 * one huge 4MB page. We chose 4MB to be consistent with what start.s
 * does, and because we have no guarantees that the PMM won't use more
 * than just the first MB.
 * 
 * We also map the first 4MB to 0xC0000000-0xC0000000+4MB, as we had in
 * start.s. In fact, after we've enabled the VMM, no address translation
 * changes, only the PD and the swap to 4KB.
 */

PRIVATE page_directory* vmm_current_directory;

void page_table_entry_add_attrib ( page_table_entry* e, uint32_t attrib )
{
    *e |= attrib;
}

void page_table_entry_del_attrib ( page_table_entry* e, uint32_t attrib )
{
    *e &= ~attrib;
}

/* Note how we make sure we clear e (in case it had another address there)
 * and f (in case it wasn't page/4kb-aligned. */
void page_table_entry_set_frame ( page_table_entry* e, uint32_t f )
{
    *e &= ~PTE_PAGE_FRAME;
    *e |= ( f & PTE_PAGE_FRAME );
}

bool page_table_entry_is_present ( page_table_entry e )
{
    return ( e & PTE_PAGE_PRESENT ) == PTE_PAGE_PRESENT;
}
bool page_table_entry_is_writable ( page_table_entry e )
{
    return ( e & PTE_PAGE_WRITE ) == PTE_PAGE_WRITE;
}

void page_directory_entry_add_attrib ( page_directory_entry* e, uint32_t attrib )
{
    *e |= attrib;
}
void page_directory_entry_del_attrib ( page_directory_entry* e, uint32_t attrib )
{
    *e &= ~attrib;
}

/* Note how we make sure we clear e (in case it had another address there)
 * and f (in case it wasn't page/4kb-aligned. */
void page_directory_entry_set_pte_address ( page_directory_entry* e, uint32_t f )
{
    *e &= ~PDE_PAGE_FRAME;
    *e |= ( f & PDE_PAGE_FRAME );
}
bool page_directory_entry_is_present ( page_directory_entry e )
{
    return ( e & PDE_PAGE_PRESENT ) == PDE_PAGE_PRESENT;
}
bool page_directory_entry_is_writable ( page_directory_entry e )
{
    return ( e & PDE_PAGE_WRITE ) == PDE_PAGE_WRITE;
}

/* Fetches any page and adds it to this page table entry (thus mapping it) */
bool vmm_alloc_page ( page_table_entry* e )
{
    uint32_t p = pmm_alloc_block ();
    if ( p == 0 )
        return false;

    page_table_entry_set_frame ( e, p );
    page_table_entry_add_attrib ( e, PTE_PAGE_PRESENT );
    page_table_entry_add_attrib ( e, PTE_PAGE_WRITE );

    return true;
}

void vmm_free_page ( page_table_entry* e )
{

    uint32_t p = PAGE_GET_PHYSICAL_ADDRESS ( *e );
    if ( p != 0 )
        pmm_free_block ( p );

    page_table_entry_del_attrib ( e, PTE_PAGE_PRESENT );
    page_table_entry_del_attrib ( e, PTE_PAGE_WRITE );
}

page_table_entry* vmm_page_table_lookup_entry ( page_table* p, uint32_t addr )
{

    if ( p )
        return &p->entries[ PAGE_TABLE_INDEX ( addr ) ];
    return 0;
}

page_directory_entry* vmm_page_directory_lookup_entry ( page_directory* p, uint32_t addr )
{

    if ( p )
        return &p->entries[ PAGE_DIRECTORY_INDEX ( addr ) ];
    return 0;
}

void vmm_switch_page_directory ( page_directory* pd )
{
    vmm_current_directory = pd;
    __asm volatile ( "mov %0, %%cr3" : : "r" ( pd ) );
}

void vmm_enable_paging ( void )
{
    uint32_t cr0;
    __asm volatile ( "mov %%cr0, %0" : "=r" ( cr0 ) );
    cr0 |= 0x80000000;
    __asm volatile ( "mov %0, %%cr0" : : "r" ( cr0 ) );
}

void vmm_disable_paging ( void )
{
    uint32_t cr0;
    __asm volatile ( "mov %%cr0, %0" : "=r" ( cr0 ) );
    cr0 &= ~0x80000000;
    __asm volatile ( "mov %0, %%cr0" : : "r" ( cr0 ) );
}

void vmm_enable_4mb_pages ( void )
{
    uint32_t cr4;
    __asm volatile ( "mov %%cr4, %0" : "=r" ( cr4 ) );
    cr4 |= 0x00000010;
    __asm volatile ( "mov %0, %%cr4" : : "r" ( cr4 ) );
}

void vmm_disable_4mb_pages ( void )
{
    uint32_t cr4;
    __asm volatile ( "mov %%cr4, %0" : "=r" ( cr4 ) );
    cr4 &= ~0x00000010;
    __asm volatile ( "mov %0, %%cr4" : : "r" ( cr4 ) );
}

page_directory* vmm_get_current_directory ( void )
{
    return vmm_current_directory;
}

void vmm_flush_tlb_entry ( uint32_t addr )
{
    /* FIXME: Do a cli and then a sti here? */
    __asm volatile ( "invlpg (%0)" : : "a" ( addr ) );
}

void vmm_map_page ( uint32_t phys, uint32_t virt )
{
    page_table* table;
    page_table_entry* page;
    
    /* Get the current page directory */
    page_directory* pageDirectory = vmm_get_current_directory ();
    
    /* Get the page directory entry for this virtual address (which points to
     * its page table*/
    page_directory_entry* e = vmm_page_directory_lookup_entry ( pageDirectory, virt );;

    /* Check we have a page table. If we don't, create it */
    if ( !page_directory_entry_is_present ( *e ) ) {
        /* Allocate a new 4kb block where we'll store the page table */
        table = ( page_table* ) pmm_alloc_block ();
        memset ( table, 0, sizeof ( page_table ) );

        /* Mark the page table entry present, writable, and make it point to
         * the table we've just allocated */
        page_directory_entry_add_attrib ( e, PDE_PAGE_PRESENT );
        page_directory_entry_add_attrib ( e, PDE_PAGE_WRITE );
        page_directory_entry_set_pte_address ( e, ( uint32_t ) table );
    } else
        table = ( page_table* ) PAGETABLE_GET_ADDRESS ( *e );

    /* table points to the address' page table */

    /* Find the page table entry where we need to map this page,
     * and map it present and writeable */
    page = vmm_page_table_lookup_entry ( table, virt );
    page_table_entry_add_attrib ( page, PTE_PAGE_PRESENT );
    page_table_entry_add_attrib ( page, PTE_PAGE_WRITE );
    
    /* The address at virt (remember it's 4kb aligned!) points to phys, as well as
     * its whole 4kb range */
    page_directory_entry_set_pte_address ( page, phys );
}

/* Used initially mainly for debugging, returns the physical address from a
 * virtual address, given the current page directory
 * Since it was built during a debugging phase, it deviates a bit from
 * the other functions' style
 */
/*PRIVATE uint32_t get_physaddr ( uint32_t virtualaddr )
{
    uint32_t pdindex = PAGE_DIRECTORY_INDEX ( virtualaddr );
    uint32_t ptindex = PAGE_TABLE_INDEX ( virtualaddr );
    page_table* t;
    page_table_entry page;

    if ( ( vmm_current_directory->entries[pdindex] & PDE_PAGE_PRESENT ) == 0 )
        screen_puts ( " !!!! Not present!!!\n" );
    t = ( page_table* ) PAGETABLE_GET_ADDRESS ( vmm_current_directory->entries[pdindex] );


    page = t->entries[ptindex];

    if ( ( page & PTE_PAGE_PRESENT ) == 0 )
        screen_puts ( " !!!! PAGE Not present!!!\n" );

    return PAGE_GET_PHYSICAL_ADDRESS ( page ) + ( virtualaddr & 0xFFF ) ;
}*/

/* We're going to enable paging, and we're not going to use 4MB pages.
 * We need to create a new page directory and map 0xC0000000 to
 * 0x0, as we did in the boot phase (it's the higher-half kernel). Notice
 * that in start.s we did this using only one page directory entry, because
 * we used 4MB pages. Since we're now using 4kb pages, we need several entries
 * to map the whole 4MB range we had mapped before (we also didn't need to map
 * the whole 4MB, but we choose to).
 * 
 * After mapping 0xC0000000 to 0x0, we need to identity-map our PMM's working area.
 * start.s already did this, which enabled the PMM to work. We need to map them too.
 * Note that currently, the PMM lives at 0xB0000000 and grows up.
 * 
 * Once that's done, we switch page directory to our new page directory, disable
 * 4 MB pages and notify the PMM that 4kb-paging has been enabled.
 */
void init_vmm ()
{
    uint32_t frame, virt;

    vmm_current_directory = ( page_directory* ) pmm_alloc_block (); /* was alloc blocks 3 */
    memset ( vmm_current_directory, 0, sizeof ( page_directory ) );

    /* Map 0xC0000000 to 0x0 for a 4MB range */
    for ( frame = 0, virt = 0xC0000000 ; frame < 0x400000; virt += 4096, frame += 4096 )
        vmm_map_page ( frame, virt );

    /* Identity map the PMM for 4MB */
    for ( frame = pmm_get_start_location(), virt = pmm_get_start_location() ; virt < pmm_get_start_location() + 0x400000; virt += 4096, frame += 4096 )
        vmm_map_page ( frame, virt );

    vmm_switch_page_directory ( vmm_current_directory );
    vmm_disable_4mb_pages();
    vmm_enable_paging();
    pmm_notify_paging_enabled();
}
