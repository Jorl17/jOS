#ifndef PMM_H
#define PMM_H
#include <stdinc.h>
#include <multiboot.h> /* For mapping all the memory locations */
/* Our Memory Management system will rely on a PMM and a VMM.
 * The PMM is the Physical Memory Manager. It manages fetching individual
 * pages and giving them to whomever needs them, generally the VMM. Once
 * the VMM has taken over the control of paging, the PMM can rely on it
 * to enlarge its 'stack' (used to manage the free/returned pages).
 * 
 * The VMM is the Virtual Memory Manager. It manages paging, and virtual
 * memory. It's responsible for mapping all the right addresses.
 * 
 * Our PMM only needs to expose functions to allocate blocks/pages,
 * as well as functions to free them. */

/* We use an abstraction called a BLOCK to represent what in x86-parlance is
 * known as a page. */
 
#ifndef PAGE_SIZE
#define PAGE_SIZE  0x1000 /* 4kb */
#define BLOCK_SIZE PAGE_SIZE
#define PAGE_MASK  0xFFFFF000 /* Mask constant to page-align an address. */
#define BLOCK_MASK PAGE_MASK
#endif

/* Start the PMM at address start. It will grow up. */
void init_pmm ( uint32_t start );

/* Get a free block/page */
uint32_t pmm_alloc_block ( bool need_mapped );

/* Get a group of free blocks/pages which are CONTIGUOUS
   Note that currently you can only get a contiguous block when the VMM
   hasn't taken over yet ('paging disabled') */
uint32_t pmm_alloc_blocks ( uint32_t n );

/* Frees a block/page, returning it to the PMM */
void pmm_free_block ( uint32_t b );

/* This function does almost the same the same as the above, but it is meant
 * to be used whenever we want to give a block/page to the PMM.
 * In particular, after VMM has been setup, the PMM has no pages to
 * work with, and any call to pmm_alloc_block will trigger an out
 * of memory warning. We should make sure that the PMM has free blocks
 * by adding all free blocks we can find to it.
 * 
 * Since it's only meant to give blocks back, it doesn't check if you're
 * trying to free an important block
 */
void pmm_give_block ( uint32_t b );


/**
 * This traverses a multiboot header given to us by GRUB to find all the available physical memory
 * and add it to the PMM.
 *
 * Return the number of bytes found in the machine.
 */
uint32_t pmm_grab_physical_memory( multiboot_t* mboot_ptr );


/* Used by the VMM to notify the PMM of paging changes
 * Do note that our kernel already boots with paging enabled
 * (see start.s). What we mean by "paging enabled" here is if the kernel
 * has entered its second paging stage, where the VMM is enabled and a new
 * page directory is effectively created (using the PMM itself).
 *
 * In summary: Paging Enabled means that the VMM has full control of paging,
 *             not simply that paging is enabled, because it always is!
 */
void pmm_notify_paging_enabled ( void );
void pmm_notify_paging_disabled ( void );

/* This returns us the start location of the PMM.
 * We need this because the VMM has to identity-map the area where the
 * PMM is running.
 */
uint32_t pmm_get_start_location(void);
#endif
