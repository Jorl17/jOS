# Makefile for JamesM's kernel tutorials.
# The C and C++ rules are already setup by default.
# The only one that needs changing is the assembler 
# rule, as we use nasm instead of GNU as.

SOURCES=start.o main.o

CFLAGS=-nostdlib -nostdinc -fno-builtin -fno-stack-protector -m32
LDFLAGS=-Tlink.ld -melf_i386
ASFLAGS=-felf
KERNEL=kernel

all: $(KERNEL) 

clean:
	rm *.o $(KERNEL)

$(KERNEL): $(SOURCES)
	ld $(LDFLAGS) -o $(KERNEL) $(SOURCES)

.s.o:
	nasm $(ASFLAGS) $<