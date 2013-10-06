; start.s -- Kernel entry point and definition of important data to be written
; to the kernel, so that GRUB can find it.
;
; This file is responsible for:
; * Enabling GRUB to find us, by placing the magic number somewhere where grub
;   should start running code.
; * Using only position-independent code, setup a higher-half kernel. That is,
;   a kernel which lives above the 2 GB barrier. Specifically, our higher-half
;   kernel is a 3/1 kernel, residing at 0xC0100000 (technically a 3/1 resides
;   at 0xC0000000, but keep reading for why we chose this address)
; * Map a region of memory (0xB0000000) to itself, so it can later be used by
;   the PMM (Physical Memory Manager) for memory management (also see pmm.h)
; * Setting up a new stack for our kernel
; * Jump to kernel_main and pass it a grub info structure
;
;
; --------------------- HOW THE HIGHER-HALF KERNEL WORKS ---------------------
; Our linker.ld script is configured so that all functions and variables are
; assumed to be from address 0xC0100000 and onwards. This means that the
; kernel really is ready to be run at 0xC0100000. We will discuss why we
; chose this address in the end of this explanation.
;
; However, GRUB can't place/load the kernel there. It loads us in the first
; megabyte of memory (because we tell it to in the linker script).
; We need to find a way to have 0xC0000000 "point" to 0x0, and
; 0xC0100000 point to 0x100000 (1mb). We do this with paging.
;
; Indeed, if we enable paging correctly, we can map the area where the kernel
; expects to be loaded to the area where it really is loaded. And since paging
; will be enabled from there on, there is no problem if it isn't physically
; there.
;
; To accomplish this, the .data section in this file has a Page Directory
; already built with all the page directory entries ready on the spot (built
; with macros during the assembly phase). When GRUB loads us, we only need to
; setup paging with this page directory. And for this, we must use
; position-independent code -- remember that the kernel expects to be loaded
; at 0xC0000000, so it can't rely on that!
;
; This page directory doesn't just map 0xC0000000. If we only mapped it, then
; when we enabled paging, we'd hit a page-fault. Why? Because since the kernel
; is effectively loaded on the lower 1mb addresses, EIP, holding the next
; instruction to be run exactly after enabling paging, will be pointing to
; code in this 1mb area, which would be unmapped! This means that we must
; also map 1mb to itself, so that the value of EIP before we enable paging
; is still valid after we enable it. The page directory, then, has two
; distinct entries to do the mapping -- one for indentity-mapping, and the
; other for mapping the kernel to the higher-half (rather, mapping the
; higher-half to the lower-addresses).
;
; Once paging has been enabled, we can jump to functions which are at
; 0xC0100000 (note that physically they are still in the 1mb area, but
; paging handles that for us and makes it really look like they're at
; 0xC0100000). Since now the kernel is where it is supposed to be
; (even if only from a virtual perspective), everything can work out of the
; box, and we no longer need position-independent code. Thus, after the jump
; to a function in the 0xC0100000 area, EIP will be holding a value valid
; in this range, and we can undo the indentity-mapping of the 1mb area,
; keeping only the mapping of our kernel.
;
; (Why 0xC0100000? Why not 0xC0000000? Let's start by remembering that no
; matter where our kernel assumes it is, it will always be loaded somewhere.
; Up until recently, we had been loading ourselves to te 1mb (0x100000) area,
; because the area 0x0-0xFFFFF is reserved for BIOS and other things.
; So our kernel will keep being loaded to 0x100000, because we set it that
; way in the linker script. We could easily choose to map 0x100000 to
; 0xC0000000 and be done with it. And, indeed, it would work. But we would
; be losing access to the first mb of memory (because it would be left
; unmapped). Yet, we use the first mb of memory for things such as VRAM, through
; memory-mapped IO. This means that it will also be beneficial for us to map
; something to the 0-1mb range. This would allow us to use it, without having
; any pointers to it. We leave the 0-1mb 'virtual' range free, but have another
; virtual area which points to it, and allows us to access it.
; 
; One such way to accomplish both these things: map the kernel and the 0-1mb
; area, is precisely by mapping them all together at once! Our kernel starts
; at 0x100000, and it goes to 0x100000+kernel_size. So, if we map
; 0x0--0x100000+kernel_size all in one go, we can solve both our problems.
; Now, of course we could just map 0x0 to any other address, even to
; 0xC0100000 itself, but if we choose to map 0x0 to 0xC0000000 (meaning that
; our kernel, which is loaded at 0x100000 is indeed mapped to 0xC0100000),
; then we have a pretty value which we can use as offset, and which doesn't
; really interfere with our programming. For instance, VRAM moves from
; 0xB8000 to 0xC00B8000. The fact that 0xC0000000 shares no non-zero bits
; with 0x100000 allows us to quickly convert from 0x0-0x100000 to 
; 0xC0000000-0xC0100000, while still remaining right above the 3gb barrier,
; which was our initial goal. It's a very personal and subjective thing,
; really.)
; 
;
; It might be relevant to note that we are using big pages to do this,
; so we use 4mb pages instead of 4kb pages. This eases coding of the
; page directory, but it also means that we end up identity-mapping 0-4mb
; and not 0-1mb. This is okay, but I thought I should point it out.
;
;
; Further reading:
; * http://wiki.osdev.org/Higher_Half_Kernel (theory)
; * http://wiki.osdev.org/Higher_Half_bare_bones (practice)
; * http://wiki.osdev.org/User:Mduft/HigherHalf_Kernel_with_32-bit_Paging (more practice)
; * http://www.jamesmolloy.co.uk/tutorial_html/6.-Paging.html (Another way to look at things)
; * http://www.brokenthorn.com/Resources/OSDev18.html (more focused on paging)
;
; ----------------------------------------------------------------------------
;
;
; Useful Magic macros
MBOOT_PAGE_ALIGN    equ 1<<0       ; Load kernel and modules on a page boundary
MBOOT_MEM_INFO      equ 1<<1       ; Provide your kernel with memory info
MBOOT_HEADER_MAGIC  equ 0x1BADB002 ; Multiboot Magic value
; NOTE: We do not use MBOOT_AOUT_KLUDGE in  MBOOT_HEADER_FLAGS
; It means that GRUB does not pass us a symbol table.
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)
; We start by storing the virtual base address of kernel space.
; Note that this is NOT where it assumes it is loaded to! This is where
; the mapping will start. We'll map 0x0 to 0xC0000000, and the kernel
; is loaded at 0x100000, and then mapped to 0xC0000000. This means that
; the offset, which we dub KERNEL_VIRTUAL_BASE is 0xC0000000
; as well as the index to the to the page directory we'll create
KERNEL_VIRTUAL_BASE equ 0xC0000000                  ; 3GB
KERNEL_PAGE_NUMBER equ (KERNEL_VIRTUAL_BASE >> 22)  ; Page directory index of kernel's 4MB PTE.
 
; The PMM will be expecting 0xB0000000 to be mapped, and in fact, indetity-mapped,
; to do its work, so we have to do it here. Do note that the only reason
; why it needs to be identity-mapped is because the VMM needs the PMM to
; give it physical addresses (so as to use them in Page Directories and Page Tables).

PMM_VIRTUAL_BASE equ 0xB0000000               
PMM_PAGE_NUMBER equ (PMM_VIRTUAL_BASE >> 22)  ; Page directory index of kernel's 4MB PTE.
  
; Since we're moving up to 0xC0100000, we're going to need to set our stack there
; as well. Thus, we're going to create our own stack, and it will be 0x4000 bytes long,
; that is 16k
STACK_SIZE equ 0x4000

global _loader                                ; Make entry point visible to linker.
extern kernel_main                            ; _main is defined elsewhere
extern build_elf_symbols_from_multiboot
 

; The .data section
section .data
align 0x1000

; In here we define our Page Directory
BootPageDirectory:
    ; The first entry of the page directory identity-maps 0-4MB. We use 4MB pages to do this,
    ; and since the range is 0-$MB, then the index of the Page Directory is 0, which is why
    ; the first entry is this one.
    ; All bits are clear except the following:
    ; bit 7: PS The kernel page is 4MB. (big pages)
    ; bit 1: RW The kernel page is read/write.
    ; bit 0: P  The kernel page is present.
    ; This gives us 0x00000083 (remember each page  directory entry is 4 bytes)
    ; This entry must be here -- otherwise the kernel will crash immediately after paging is
    ; enabled because it can't fetch the next instruction! It's ok to unmap this page later.
    dd 0x00000083
    
    ; Add null pages until we reach the PMM page number
    times (PMM_PAGE_NUMBER - 1) dd 0                       ; Pages before PMM space.
    dd 0xB0000083     ; This page directory entry identity-maps the 4MB starting at 0xB0000000
    
    ; Null pages until we reach the page where our kernel will live, and
    ; which we'll have to map. FIXME: If the kernel ever gets really big (bigger than 4MB),
    ; It might be left unmapped!!
    times (KERNEL_PAGE_NUMBER - PMM_PAGE_NUMBER - 1) dd 0  ; Pages before kernel space.
    dd 0x00000083     ; This page directory entry defines a 4MB page containing the kernel.
    
    ; Add the remaining pages until we've filled the Page Directory
    ; (Remember a page directory is an array of 1024 page tables)
    times (1024 - KERNEL_PAGE_NUMBER - 1) dd 0  ; Pages after the kernel image.
 
; begin of the .text/CODE section 
section .text
align 4
; We start with the multi boot header. Notice that the section name is irrelevant,
; it is only here for clarity, as it isn't used anywhere
MultiBootHeader:
   dd  MBOOT_HEADER_MAGIC        ; GRUB will search for this value on each
                                 ; 4-byte boundary in the kernel file
   dd  MBOOT_HEADER_FLAGS        ; How GRUB should load your file / settings
   dd  MBOOT_CHECKSUM            ; To ensure that the above values are correct


; setting up entry point for linker
start equ (_loader - 0xC0000000)
global start

; This is where the fun begins!
_loader:
    ; This code here is for us to have a symbol table built and stored by the kernel
    ; We do it here because we still haven't enabled paging. FIXME: Check this
    ;push ebx ; Pus
    ;call build_elf_symbols_from_multiboot
    
    ; Since we haven't setup paging yet, and our linker assumes we'll be loaded at
    ; 0xC0100000 (even though we're loaded at 1MB), we must use position-independent
    ; code and rely on physical addresses instead of virtual ones.
    ;
    ; The whole point is that of enabling paging so that this is no longer a problem.
    
    ; The linker has this mapped to the virtual address, but it is loaded in
    ; another address. This address is found by subtracting the KERNEL_VIRTUAL_BASE
    ; from the address that the linker thinks BootPageDirectory is at.
    ; Thus, we're loading ecx with the address of the Page Directory
    mov ecx, (BootPageDirectory - KERNEL_VIRTUAL_BASE)
    mov cr3, ecx                                ; Load Page Directory Base Register.
 
    mov ecx, cr4
    or ecx, 0x00000010                          ; Set PSE bit in CR4 to enable 4MB pages.
    mov cr4, ecx
 
    mov ecx, cr0
    or ecx, 0x80000000                          ; Set PG bit in CR0 to enable paging.
    mov cr0, ecx
 
    ; We've just enabled paging, and eip holds the physical address of this command.
    ; In fact, that's why we indentity-mapped in the first place!It is so that we can run
    ; the next instructions.
    ;
    ; So, since the kernel now lives in the upper Gb, that's where we should go to
    ; Thus, we load the address of StartInHigherHalf and jump there!
    ; (This address is in the 0xC0100000 area)
    lea ecx, [StartInHigherHalf]
    jmp ecx                                                     ; NOTE: Must be absolute jump!
 
StartInHigherHalf:
    ; We don't need the identity-mapped first 4MB of physical addaddress space. We don't need it
    ; anymore, since this function is to be reached once we've already jumped to the top 1GB
    
    ; So we just zero it out of our PageDirectory, and invalidate that page (to flush the TLB/cache)
    mov dword [BootPageDirectory], 0
    invlpg [0]
 
    ; We're finally done with converting addresses and whatnot. Paging is enabled,
    ; we've removed the identity map, and all we need to do is setup a stack for ourselves.
    ;
    ; We've setup a label named "stack" after which we will have a stack of size STACK_SIZE.
    ; Remember it grows down, so if we exceed the 16 kb, we get to the section that comes before
    ; the stack.
    mov esp, stack+STACK_SIZE           ; set up the stack (make esp point to the stack boundary+the stack size, since it grows down)
 
    ; pass Multiboot info structure -- WARNING: This is a physical address and may not be
    ; in the first 4MB!
    push    ebx                   ; Load multiboot header location
    mov     ebp, 0                ; Start ebp with 0 so that we can do stack traces until this very spot
 
    call  kernel_main                  ; call kernel
    hlt                                ; halt machine should kernel return
 
 
section .bss
align 32
stack:
    resb STACK_SIZE      ; reserve 16k stack on a quadword boundary (the boundary makes it possible to run in 64-bit mode too)
