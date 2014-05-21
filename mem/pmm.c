#include "pmm.h"
#include <kpanic.h>
#include "vmm.h" /* The PMM uses the VMM after paging has been enabled */
#include <screen.h>

/* FIXME: Right now, the PMM's functionality when paging has been enabled
 * is mostly unused, untested and even questionable! It traces back to
 * JamesM's tutorial.. */
#define PMM_STACK_ADDR 0xD1000000 /* FIXME: When we get here, paging is already enabled! We need to map this! */

PRIVATE uint32_t pmm_curr_location;
PRIVATE uint32_t pmm_start_location;

PRIVATE uint32_t pmm_stack_loc = PMM_STACK_ADDR;
PRIVATE uint32_t pmm_stack_max = PMM_STACK_ADDR;

PRIVATE bool pmm_paging_active = false;

void init_pmm ( uint32_t start )
{
    if ( (start & BLOCK_MASK) == start)
        pmm_start_location = pmm_curr_location = start;
    else
        pmm_start_location = pmm_curr_location = ( start + BLOCK_SIZE ) & BLOCK_MASK;
}

uint32_t pmm_get_start_location ( void )
{
    return pmm_start_location;
}

uint32_t pmm_alloc_block ( bool need_mapped )
{
    if ( pmm_paging_active && !need_mapped ) {
        uint32_t* stack;
        /* Quick sanity check. */
        if ( pmm_stack_loc == PMM_STACK_ADDR )
            kpanic ( "Error:Request for unmapped page, but we don't have any!" );

        /* Pop a page off of the stack */
        pmm_stack_loc -= sizeof ( uint32_t );
        stack = ( uint32_t* ) pmm_stack_loc;

        return *stack;
    } else {
        if ( pmm_curr_location + BLOCK_SIZE >= PMM_STACK_ADDR )    {
            kpanic ( "Error: Request for mapped page would get into stack's page!" );
        }
        return
                pmm_curr_location += BLOCK_SIZE;
    }
}

uint32_t pmm_alloc_blocks ( uint32_t n )
{
    if ( pmm_paging_active )
        kpanic ( " Error:out of memory for larger allocation." );

    return pmm_curr_location += ( n * BLOCK_SIZE );
}

void pmm_give_block ( uint32_t b )
{
    /* If the stack is at its limit, we wil use the new page
     * to enlarge the stack itself */
    if ( pmm_stack_max <= pmm_stack_loc ) {
        /*vmm_map_page(pmm_stack_max,b);*/
        vmm_map_page(b,pmm_stack_max);
        /*map ( pmm_stack_max, p, PAGE_PRESENT | PAGE_WRITE );*/
        pmm_stack_max += BLOCK_SIZE;
    } else {
        /* We have space on the stack, so we just push the address
         * of this page onto it. */
        uint32_t* stack = ( uint32_t* ) pmm_stack_loc;
        *stack = b;
        pmm_stack_loc += sizeof ( uint32_t );
    }
}

void pmm_free_block ( uint32_t b )
{
    /* We can't free anything below pmm_curr_location, because
     * those were used by the VMM before it took control. If we
     * free it, we might break page directories and page entries.
     * In summary: these pages probably contain essential data
     * to the VMMs functioning */
    if ( b < pmm_curr_location ) return;

    pmm_give_block(b);
    
}

void pmm_notify_paging_enabled ( void )
{
    pmm_paging_active = true;
}
void pmm_notify_paging_disabled ( void )
{
    pmm_paging_active = false;
}

uint32_t pmm_grab_physical_memory( multiboot_t* mboot_ptr )
{
    uint32_t i, j, total_bytes = 0;
    mmap_entry_t *me;    

    /* Traverse all memory ranges that GRUB reports to us. We traverse them in multiples of the page size */
    for (i = mboot_ptr->mmap_addr, me = (mmap_entry_t*) i ;
         i < mboot_ptr->mmap_addr + mboot_ptr->mmap_length;
         i += me->size + sizeof (uint32_t), me = (mmap_entry_t*) i)
        if (me->type == 1)
            for (j = me->base_addr_low; j < me->base_addr_low+me->length_low; j += BLOCK_SIZE, total_bytes += BLOCK_SIZE)
                pmm_give_block (j);

    return total_bytes;
}


