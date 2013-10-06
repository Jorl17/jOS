#ifndef VMM_H
#define VMM_H
#include <stdinc.h>
/* NOTE: See pmm.h's documentation first!
 * 
 * Our VMM is a somehwat complex piece of programming responsible for managing
 * paging and virtual memory. This documentation will attempt to explain x96
 * paging, but will most likely fail to do so. However, if used together with
 * the recommended reading, it might be helpful for those looking to understand
 * the innerworkings of paging for the x86.
 * 
 * WHAT DOES PAGING DO?
 * Paging is used both for virtual memory as a way to map virtual addresses to
 * physical addresses, and as a way to established privileged sections of memory
 * where only certain priviliged (kernel-mode) code can operate.
 * 
 * When we speak of virtual addresses and physical addresses, we mean that
 * all addresses that go through the CPU, when paging is enabled, are considered
 * virtual, because they don't really mean those addresses in the physical memory.
 * For instance, if paging is enabled, we might map address 0xAB to address
 * 0xD3C if we want to. Then, whenever the cpu has to access 0xAB, it instead
 * accesses 0xD3C. The 0xAB is the 'virtual address', and 0xD3C is the 'physical
 * address', the one that the CPU really accesses.
 * 
 * Note that when paging is enabled, all addresses are virtual by definition.
 * Even if the instruction pointer is pointing to some instruction, it will be
 * converted to its corresponding physical address. Thus, when paging is enabled,
 * the applications have no way to realize it. It's transparent. It just takes
 * a while for the Operating System to set up paging -- making all the mappings
 * (it has to decide what addresses map to what addresses -- he might even decide
 * to map several addresses to the same one, so as to achieve 'shared memory'!)
 * 
 * HOW IS THIS DONE ON THE x86?
 * 
 * On the x86, this is done by considering that the address space 0-4gb is
 * split on pages. These pages can either be 4kb or 4mb, but we'll mostly deal
 * with the 4kb. Since they're 4kb, the block 0-4kb is considered a "page"
 * (do note that later on we'll more precisely define the notion of 'page').
 * However, the range 1-0x1001 (1 byte to 4kb+1 byte) is not, because these
 * pages can't be placed anywhere in memory! They must be aligned on 4kb
 * boundaries. To sum up, the x86 considers that the whole memory can be treated
 * as fixed-size blocks on fixed boundaries, all of 4kb. As if you were splitting
 * chocolate in equal pieces, but ALWAYS starting from the top (can't split it
 * halfway).
 * 
 * We've stated that one of the functions of paging is mapping virtual addresses
 * to physical addresses. The x86 does this by these blocks called pages. It
 * doesn't map one address to another -- it maps 4kb blocks to other 4kb blocks.
 * So it might map the 0x0-0x1000 region to the 0xA00000-0xA0100 region, but not its
 * individual addresses. In this context, when we talk about the physical groups
 * of 4kb, we call them Frames or Page Frames. A Page Frame is a group of 4kbytes
 * aligned on a 4kbyte boundary in physical memory -- one of those blocks we
 * split up before.
 * 
 * So what's the difference between a page and a frame? A frame is the actual
 * block of 4kb in physical memory. A page is the exact same thing, but dealing
 * with virtual addresses. When you mess with the contents of  the
 * 'page at 0xB0000000', you are messing with the contents of a frame which
 * might be somewhere else. Page is the virtual 4kbyte block, which you try to
 * access, and frames are the ones that pages are mapped to. When we we
 * map 0xB0000000 to 0x0, we map the page starting at 0xB0000000 to the
 * frame at 0x0. Each page has an associated frame that it maps to, but
 * some frames might be left comletely unmapped -- and notice that many
 * pages can map to the same frame at once!
 * 
 * As for the privilege needs, the x86 allows us to define the priviliged
 * level needed to access a given page. If code attempts to write to a
 * read-only page, we get a Pafe Fault due to an invalid write.
 * 
 * We might also get page faults when a mapping is missing. This means that
 * we try to access, for intance, 0xB0000000, if we haven't created a
 * page for it, so it isn't mapped to any frame, it will page fault.
 * It's a 'page fault because a page is at fault to tell the CPU what
 * 0xB0000000 maps to!
 * 
 * Simple, right? Yeah, I know it isn't, but I'm doing my best.
 * 
 * So now we know about pages, mapping to frames. They are usually 4kbytes
 * in size. Good, so how *do* we tell the CPU what the mappings and privileges
 * are?
 * 
 * Since we've split up the memory in groups of 4kbytes, we could just create
 * a table which says that 'page 0 maps to frame X', 'page 1 maps to frame Y',
 * etc. To cover the 4GB range of 32 bit machines with 4Kbyte pages, we'd need
 * 4GB/4KB=1048576 entries. If we use 32 bits (can't use 16, not enough for all
 * those numbers), we'd need 1048576 * 4 bytes = 4 Mbytes to map the entire
 * 4GB range with paging. That's big! It might seem small, but where memory is
 * at a price, 4MByte is a lot!
 * 
 * So the folks at Intel devised a hierarchichal system which eases this in
 * terms of memory, but makes it harder in terms of complexity.
 * 
 * They devised a system where the MMU (Memory Management Unit -- responsible
 * for converting virtual to physical addresses) decomposes an address and
 * uses that decomposition.
 * 
 * Suppose you try to access the virtual address
 * 0x345BCDEF = 110100010110111100110111101111.
 * 
 * The MMU splits up that address in 3 parts:
 * 
 * 0011010001        0110111100              110111101111 = 0x345BCDEF in binary
 * AAAAAAAAAA        BBBBBBBBBB              CCCCCCCCCCCC
 * directory index   page table index        offset into page
 *
 * The directory index is used to index a table called a PD (Page Directory).
 * When we access that table at that index, we'll find a Page Directory Entry
 * (PDE), which gives us a pointer to another table, which is called a PT
 * (Page Table). If we use the page table index to access an index at this table,
 * we'll find a Page Table Entry (PTE), which in turn gives us a pointer to
 * the beginning of a frame. Thus, with the first two parts of the address,
 * we can know what frame it belongs to (remember: frame is the PHYSICAL memory,
 * so we've already converted most of this VIRTUAL address to a PHYSICAL address).
 * How do we convert the remaining bits? It's simple. We already have the frame,
 * we only need the offset in that frame (address 0xB00000300 has offset 300 to
 * frame 0xB0000000). To do this, we only need add (or 'or') the 'offset into page'.
 * 
 * It's a brilliant scheme! You can think of it as a simpler scheme:
 * AAAAAAAAAAAAAAAAAAAACCCCCCCCCCCC
 * 
 * where AAAAAAAAAAAAAAAAAAAA allows us to go to an array lookup what
 * frame that address is mapped to, and where CCCCCCCCCCCC gives us the offset
 * to that address. The difference to intel's method is that they divided
 * the AAAAAAAAAAAAAAAAAAAA bit further more, but it's still the same logic:
 * You take an address, find the frame to which it belongs to (using the most
 * signifficant bits, which you can do because frames are 4kb on a 4kb boundary,
 * so the CCCCCCCCCCCC part is always 0 at the beginning of a frame), then you
 * add the offset to it. The offset a virtual address to page X is the same
 * as the offset of the corresponding physical address to the corresponding
 * frame.
 * 
 * It's hard to get it, but once you understand it, you'll admire how brilliant it
 * is!
 * 
 * What about the privilege information? Well, notice that I mentioned that
 * we use PDE's that give us pointers to Page Tables and PTE's that give us
 * pointers to Frames. The PDEs (Page Directory Entries) and PTEs (Page Table
 * Entries) do more than just that! Besides the pointers, they use their bits
 * to define access priviliges, as well as some Intel options related to caching..
 * and we can eve use some leftover bits for our own house-keeping!
 * 
 * Let's now take a look at these structures more carefully.
 * 
 * A Page Table Entry (PTE) is the last structure we access, and it is a pointer
 * to a frame, with additional info. It's not a structure in the conventional C-speak,
 * its a 32-bit value whose bits are as follows:
 * 
 * -------------------------------------------------------------------------------------------------
 * |  Frame Address | Available | Reserved | Dirty | Access | User/Kernel Flag | RW Flag | Present | 
 * |     20 bits    |  3 bits   |  2 bits  | 1 bit |  1 bit |      1 bit       |  1 bit  |  1 bit  |
 * -------------------------------------------------------------------------------------------------
 * 
 * As we would expect, a PTE holds the frame address concerning all addresses that
 * lead to this entry. It does this by storing the 20 most significant bits. We
 * know that since pages/frames are 4kbyte aligned, the remaining 12 bits are zero.
 * 
 * This leaves 12 bits of the PTE for other options. Of those 12 bits:
 * -> The "Available" bits are free for us to use for what we want
 * -> The "Reserved" bits are reserved and we can't use them
 * -> The "Dirty" bit is set by the Proessor/Hardware when a page is writen to
 *    (the MMU can do this for us) FIXME: Is it the MMU?
 * -> The "Access" bit is the same as the dirty bit, but for all access operations
 * -> The "User/Kernel Flag" is used to decide if the page can be accessed by user
 *    mode or not. If it is set to 1, then it's a user mode page and it can only
 *    be read or writen to by kernel-mode code.
 * -> The "RW Flag" is used to mark the page as read-only. 0 means read-only, 1
 *    means writeable. NOTE that we need to set this for most of our pages!!
 * -> The "Present" bit is used to tell the CPU if this entry is valid, that is,
 *    contains a valid frame address, or not. If it's 0, the processor ignores all
 *    other 31 bits of the PTE, which is very useful for us to do some bookkeping.
 * 
 * So that's what a PTE is. Now, remember that the PTE is accessed with a portion
 * of the virtual address called the "Page Table Index". This is an index to a table
 * of PTEs, called the "Page Table".
 * 
 * A PT (Page Table) is an array of 1024 PTEs that must be 4kbyte aligned.
 * That means its 4KB in size. How useful, that if fits inside a neat little page
 * by itself -- and how neat that it is itself aligned on 4kbyte boundaries, and,
 * thus, has the lower 12 bits zeroed out.
 * Yes that's all a PT is. Nothing fancy, just an array of PTEs. Notice that since
 * we have 1024 entries, each PT can cover 4MB of memory. So if we have a system
 * with only 16MB of memory, we can store its mappings in 3 PT, which ocuupy
 * 16 KB (remember each page table is 4KB?). So instead of needing those 4MB we
 * originally though we'd need (when we thought about our indexing paging scheme),
 * we only need 16 KB! That's a BIG improvement. Notice that if we needed to use 4MB
 * we'd be using up a quarter of memory!!
 * 
 * Now we are going to build another table to allow us to index these "groups" of
 * 4MB (which are groups of 4KB, which are indivisable). 
 * 
 * Let's focus on the other index we saw. It's the "Page Directory Index", and it is
 * used to access a "Page Directory Entry".
 * 
 * A PDE (Page Directory Entry) is very similar to a PTE, except instead of storing
 * the address of a frame, it stores the address of a Page Table (PT). It can do it
 * because since the PT is 4kbyte aligned, we know that its lower 12 bits are zero,
 * so we only need to store the 20 most significant bits. The structure of a PDE is
 * as follows (notice how it is very very very similar to a PTE):
 * 
 * --------------------------------------------------------------------------------------------------------------------------------------
 * | PT Address | Available |   G    | Page Size | Reserved | Access | PCD/Cache | Write-Through | User/Kernel Flag | RW Flag | Present | 
 * |  20 bits   |  3 bits   | 1 bit  |   2 bits  |   1 bit  |  1 bit |   1 bit   |     1 bit     |      1 bit       |  1 bit  |  1 bit  |
 * --------------------------------------------------------------------------------------------------------------------------------------
 * 
 * Again, the 20 most signifficant bits are used to build a pointer, this time not
 * to indicate a frame, but the location of a PT. Again, this is possible because
 * we know that the remaining 12 bits of the PT address are 0 (a PT is page-aligned
 * by definition).
 * 
 * The remaining bits are as follows:
 * -> "Available": free for us to use for what we want
 * -> "G", "Global Page": Unused
 * -> "Reserved": Reserved by Intel
 * -> "Access": is set by the Proessor/Hardware when a page is writen to
 * -> "PCD/Cache": Enable or disable caching of pages (we'll discuss it later)
 *    0 means caching disabled.
 * -> "Write-Through": Toggle write back or write-through caching. 0 means
 *    write-back, 1 means write-through
 * -> "User/Kernel Flag": is used to decide if the page can be accessed by user
 *    mode or not. If it is set to 1, then it's a user mode page and it can only
 *    be read or writen to by kernel-mode code.
 * -> "RW Flag": is used to mark the PT as read-only. 0 means read-only, 1
 *    means writeable. NOTE that we need to set this for most of our PTs!!
 * -> "Present": used to tell the CPU if this entry is valid, that is,
 *    contains a valid PE address, or not. If it's 0, the processor ignores all
 *    other 31 bits of the PDE, which is very useful for us to do some bookkeping.
 * 
 * These PDEs all come together in a Page Directory which, as you've probably
 * guessed by the similarities between PTEs and PDEs, is an array of 1024 PDEs,
 * thus fitting in a page. It also has to be page-aligned.
 * 
 * To cover the whole 4GB, we arrange 1024 PDEs. Why? Because each PT
 * covers 4MB of memory, so 1024 of them cover 4GB! Easy right?
 * 
 * So for paging to work, we need a PD, which has PDE, that point to PTs,
 * which have PTEs which themselves point to frames.
 * 
 * SO HOW DO WE ENABLE PAGING?
 * 
 * To enable paging, we must place the PHYSICAL address of a PD (I can't stress
 * how much it is important that the address of the PD is the physical one!!)
 * in the cr4 register. Then we can enable paging by changing a bit in the cr3
 * register. As soon as we enable paging, it's on! Be careful, because EIP will
 * not change its value and might now be pointing to a virtual address that has
 * no mapping, or an incorrect mapping! Always make sure that EIP is pointing to
 * where you want it to right before you enable paging or change something in
 * a PD, PDE, PT or PTE!
 * 
 * It's that simple! Load the PD to cr4 and then flip the cr3 Paging bit to 1!
 * 
 * HOW IS PAGING MANAGED IN jOS?
 * 
 * The answer to this question is documented in vmm.c
 * 
 * WHAT IS IDENTITY MAPPING?
 * 
 * A term which often appears when talking about paging, Identity Mapping is the
 * act of mapping a page to its corresponding physical frame. That is, for
 * instance, to map the 0x0-0x1000 to itself. When we identity-map a region,
 * all virtual addresses in that area are the same as physical addresses,
 * thus working even with paging disabled.
 * 
 * WHAT IS THE TLB?
 * 
 * The TLB is the Translation Lookaside Buffer, which is a cache used for
 * Virtual to Physical address translation. It makes this translation faster
 * by caching often-used PTEs. However, it must be informed when we change
 * a PTE with the INVLPG instruction. This means that whenever we change a
 * PTE after originally adding it, we have to INVLPG-it.
 * 
 * ONE FINAL NOTE
 * 
 * The x86 has support for 4MB pages. In this paging scheme, we don't use
 * PTEs or PTs. We map the PDEs directly to 4MB frames, and some (not all)
 * of these bits change. Refer to the documentation for further info.
 * 
 * Note, however, that our OS boots up with 4MB pages enabled (see start.s)
 * 
 * FURTHER READING
 * 
 * -> http://www.brokenthorn.com/Resources/OSDev18.html (Excellent tutorial
 *    which teaches paging in theory and in practice, but excells at the
 *    former.)
 * -> http://wiki.osdev.org/Setting_Up_Paging (Practical info)
 * -> http://littleosbook.github.io/#paging (Brilliant theory regarding
 *    paging on the x86)
 * -> http://www.rcollins.org/ddj/May96/ (Excellent documentation in what
 *    concerns 4MB pages)
 * -> http://wiki.osdev.org/Paging (More theoretical background with a little
 *    bit of a practical POV)
 * -> http://pdos.csail.mit.edu/6.828/2008/readings/i386/s05_02.htm (light
 *    theory for those still in need)
 */ 

/* Define bitmasks to check the Present, Write, User, Write-Through, Not-Cacheable,
   accessed, dirty bits, as well as the reserved, available (for use) and frame
   parts of the Page Table Entry (PTE). */
#define PTE_PAGE_PRESENT          1       /* 00000000000000000000000000000001 */
#define PTE_PAGE_WRITE            2       /* 00000000000000000000000000000010 */
#define PTE_PAGE_USER             4       /* 00000000000000000000000000000100 */
#define PTE_PAGE_WRITETHROUGH     8       /* 00000000000000000000000000001000 */
#define PTE_PAGE_NOT_CACHEABLE 0x10       /* 00000000000000000000000000010000 */
#define PTE_PAGE_ACCESSED      0x20       /* 00000000000000000000000000100000 */
#define PTE_PAGE_DIRTY         0x40       /* 00000000000000000000000001000000 */
#define PTE_PAGE_RESERVED      0x180      /* 00000000000000000000000110000000 */
#define PTE_PAGE_AVAIL         0xE00      /* 00000000000000000000111000000000 */
#define PTE_PAGE_FRAME         0xFFFFF000 /* 11111111111111111111000000000000 */

/* Define bitmasks to check the Present, Write, User, Write-Through, Not-Cacheable,
   accessed, dirty bits, as well as the reserved, available (for use) and frame
   parts of the Page Table Entry (PTE). */
#define PDE_PAGE_PRESENT          1       /* 00000000000000000000000000000001 */
#define PDE_PAGE_WRITE            2       /* 00000000000000000000000000000010 */
#define PDE_PAGE_USER             4       /* 00000000000000000000000000000100 */
#define PDE_PAGE_WRITETHROUGH     8       /* 00000000000000000000000000001000 */
#define PDE_PAGE_NOT_CACHEABLE 0x10       /* 00000000000000000000000000010000 */
#define PDE_PAGE_ACCESSED      0x20       /* 00000000000000000000000000100000 */
#define PDE_PAGE_DIRTY         0x40       /* 00000000000000000000000001000000 */
#define PDE_PAGE_4MB           0x180      /* 00000000000000000000000110000000 */
#define PDE_PAGE_AVAIL         0xE00      /* 00000000000000000000111000000000 */
#define PDE_PAGE_FRAME         0xFFFFF000 /* 11111111111111111111000000000000 */

/* Typedefs for a PTE and a PDE. They're just 32-bit values */
typedef uint32_t page_table_entry;
typedef uint32_t page_directory_entry;

/* Add an attribute to this PTE. You can choose one of the PTE_PAGE_* macros
 * above as the attrib argument. For instance, to mark a PTE as present and
 * write-enabled.
 * page_table_entry_add_attrib ( &e, PTE_PAGE_PRESENT );
 * page_table_entry_add_attrib ( &e, PTE_PAGE_WRITE );
 * 
 * Assuming that e is the 32-bit Page Entry.
 */ 
void page_table_entry_add_attrib ( page_table_entry* e, uint32_t attrib );

/* This deletes an attribute added with page_table_entry_add_attrib. */
void page_table_entry_del_attrib ( page_table_entry* e, uint32_t attrib );

/* This assigns a frame to a PTE. That is, it makes this PTE concern that
 * 4kb group of physical memory. */
void page_table_entry_set_frame ( page_table_entry* e, uint32_t frameaddress );

/* Functions to check various attributes of a PTE */
bool page_table_entry_is_present ( page_table_entry e );
bool page_table_entry_is_writable ( page_table_entry e );

/* These functions are exactly the same as the page_table_entry_* family of
 * functions, except they concern PDs. */
void page_directory_entry_add_attrib ( page_directory_entry* e, uint32_t attrib );
void page_directory_entry_del_attrib ( page_directory_entry* e, uint32_t attrib );
void page_directory_entry_set_pte_address ( page_directory_entry*, uint32_t pte_address );
bool page_directory_entry_is_present ( page_directory_entry e );
bool page_directory_entry_is_writable ( page_directory_entry e );

/* Useful macros used for minimal abstraction */
#define PAGES_PER_TABLE        1024
#define PAGE_TABLES_PER_DIR    1024
#define PTABLE_ADDR_SPACE_SIZE 0x400000
#define DTABLE_ADDR_SPACE_SIZE 0x100000000

/* These two macros allow us to get the indexes we need to access
 * the right PDE and PTE. */
#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3FF)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3FF)

/* Gets a physical address from a PTE, leaving room for us to add the offset */
/* FIXME: If we ever change to 64-bit, using ~0xFFF might be better than 
 * 0xFFFFF000!! */
#define PAGE_GET_PHYSICAL_ADDRESS(x) ((x) & 0xFFFFF000)

/* Get the address of a PT from a PDE. Note how all addresses have the lower
 * 12 bits zeroed out -- that's the effect of being page/4kb-aligned! */
#define PAGETABLE_GET_ADDRESS(x) ((x) & 0xFFFFF000)

/* The PMM has these too. It also further documents them */
#ifndef PAGE_SIZE
#define PAGE_SIZE  0x1000 /* 4kb */
#define BLOCK_SIZE PAGE_SIZE
#define PAGE_MASK  0xFFFFF000 /* Mask constant to page-align an address. */
#define BLOCK_MASK PAGE_MASK
#endif

/* This defines a PT, which is, as we've seen, nothing more than an array
 * of 1024 PTEs. */
typedef struct _page_table {

    page_table_entry entries[PAGES_PER_TABLE];
}  __attribute__((packed)) page_table;

/* This defines a PD, which is, as we've seen, nothing more than an array
 * of 1024 PDEs. */
typedef struct _page_directory {

    page_directory_entry entries[PAGE_TABLES_PER_DIR];
}  __attribute__((packed)) page_directory;

/* Allocate a random physical page to this PTE. The VMM asks the PMM for
 * a free page (located anywhere in memory) and assigns it to the PTE,
 * marking ir PRESENT and WRITE-able. It's useful if we want to map
 * just about anypage to a specific address, which is what we'll do
 * most of the time. */
bool vmm_alloc_page (page_table_entry* e);

/* This undoes what vmm_alloc_page does, returning the page to the PMM
 * and marking the PTE as NOT PRESENT */
void vmmn_free_page ( page_table_entry* e );

/* This function returns a pointer to the PDE used to map the virtual address
 * 'addr' to a physical address. All it does is use PAGE_DIRECTORY_INDEX
 * to find the index into the supplied PD, and then return a pointer to it. */
page_directory_entry* vmm_page_directory_lookup_entry ( page_directory* p, uint32_t addr );

/* This function return a pointer to the PTE used to map the virtual address
 * 'addr' to a physical address. All it does is use the PAGE_TABLE_INDEX to find
 * an index into the supplied PT, and then return a pointer to it. Notice how
 * usually one must first use vmm_page_directory_lookup_entry with the same addr
 * to find the required PTE */
page_table_entry* vmm_page_table_lookup_entry ( page_table* p, uint32_t addr );

/* Switch the page directory to the supplied PD. The VMM will keep track of
 * this address, and you'll be able to find it with vmm_get_current_directory()
 */
void vmm_switch_page_directory (page_directory* pd);

/* Get a pointer to the current PD. As the API developer we do not guarantee
 * that this address is the real physical address of the PD, merely that
 * it is mapped to it. (Though most probably it is identity-mapped) */
page_directory* vmm_get_current_directory ( void );

/* Enable and disable paging. NOTE that currently we already boot (see start.s)
 * with paging enabled! */
void vmm_enable_paging ( void );
void vmm_disable_paging ( void );

/* Flush this TLB entry. We use this whenever we change a PTE or PDE (FIXME: needed for PDEs? */
void vmm_flush_tlb_entry ( uint32_t addr );

/* This maps a virtual page to a physical frame. Note that both virt and phys
 * have to be page/4kb-aligned. The function makes sure that the current PD
 * has the necessary PDEs, creating any necessary PTs and PTEs. It's a somewwhat
 * complex process which can be transparently done with it. */
void vmm_map_page ( uint32_t phys, uint32_t virt );

/* The all mighty function to initialize the VMM. See vmm.c to know
 * what the VMM does when it starts up */
void init_vmm ();
#endif
