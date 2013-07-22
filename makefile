# Makefile for JamesM's kernel tutorials.
# The C and C++ rules are already setup by default.
# The only one that needs changing is the assembler 
# rule, as we use nasm instead of GNU as.

SOURCES=start.o main.o x86/x86.o screen.o gdt.o gdt_s.o mem.o idt.o idt_s.o

# Optimizing is always dangerous...
OPTIMIZATION_FLAGS=#-O3 -funroll-loops

AGGRESSIVE_FLAGS=-Wall -Wextra -ansi -pedantic -pedantic-errors -Werror -D__JOS_PEDANTIC
INCLUDES=-Ix86/ -I.
CFLAGS=-nostdlib -nostdinc -fno-builtin -fno-stack-protector -m32 $(AGGRESSIVE_FLAGS) $(OPTIMIZATION_FLAGS) $(INCLUDES)
LDFLAGS=-Tlink.ld -melf_i386
ASFLAGS=-felf
KERNEL=kernel

CC=gcc #gcc or clang...or something completely different!

all: $(KERNEL) 	

clean:
	rm $(SOURCES) $(KERNEL)

$(KERNEL): $(SOURCES) link.ld
	ld $(LDFLAGS) -o $(KERNEL) $(SOURCES)

.s.o:
	nasm $(ASFLAGS) $<

link.ld:
	#Nothing. We just want to make sure that if link.ld changes, the whole
	# thing gets linked again.
