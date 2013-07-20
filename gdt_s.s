global gdt_set:function gdt_set.end-gdt_set ; Allows the C code to call gdt_flush().

; Remember that the segments hold byte INDEXES to the GDT table. This, if the
; DATA selector is the second entry (starting at byte 0x08), we want to
; load AX,DS,ES,FS,GS and SS (the DATA segments) with 0x08. This is no magic
; trick, it really is as simple as that: the segments hold indexes to the GDT
; table.
;
gdt_set:
    mov eax, [esp+4]  ; Get the special pointer to the GDT, passed as a param\eter.
    lgdt [eax]        ; Load the new special GDT pointer

    mov ax, 0x10      ; 0x10 is the offset in the GDT to our data segment (3rd entry = 16th byte)
    mov ds, ax        ; Load all data segment selectors with this data segment
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; We can't do a mov to set the CS register...
    ; The way used to set the CS register is doing a FAR/LONG jump in which
    ; we set the value of the CS register. Example: JMP 0xACDC:0x5578
    ; loads CS with 0xACDC and jumps to 0x5578 (relative to the new CS)
    ; Thus, to load CS, we're going to jump to the next instruction, while doing
    ; a far jump (setting CS=0x08).
    ;
    jmp 0x08:.set     ; 0x08 is the offset to our code segment.
.set:
    ret
.end:
