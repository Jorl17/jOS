# Makefile for JamesM's kernel tutorials.
# The C and C++ rules are already setup by default.
# The only one that needs changing is the assembler 
# rule, as we use nasm instead of GNU as.

SOURCES=start.o main.o

# Optimizing is always dangerous...
OPTIMIZATION_FLAGS=#-O3 -funroll-loops

AGGRESSIVE_FLAGS=-Wall -Wextra -ansi -pedantic -pedantic-errors -Werror -D__JOS_PEDANTIC
CFLAGS=-nostdlib -nostdinc -fno-builtin -fno-stack-protector -m32 $(AGGRESSIVE_FLAGS) $(OPTIMIZATION_FLAGS)
LDFLAGS=-Tlink.ld -melf_i386
ASFLAGS=-felf
KERNEL=kernel

CC=gcc #gcc or clang...or something completely different!

all: $(KERNEL) 

clean:
	rm *.o $(KERNEL)

$(KERNEL): $(SOURCES) link.ld
	ld $(LDFLAGS) -o $(KERNEL) $(SOURCES)

.s.o:
	nasm $(ASFLAGS) $<

link.ld:
	#Nothing. We just want to make sure that if link.ld changes, the whole
	# thing gets linked again.
