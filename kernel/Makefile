ASM=nasm
ASMFLAGS=-f elf
GCC=gcc
GCCFLAGS=-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs
LD=ld
LDFLAGS=-T../linker.ld

all:
	@$(GCC) $(GCCFLAGS) assert.c -o assert.o
	@$(GCC) $(GCCFLAGS) gdt.c -o gdt.o
	@$(GCC) $(GCCFLAGS) idt.c -o idt.o
	@$(GCC) $(GCCFLAGS) initrd.c -o initrd.o
	@$(GCC) $(GCCFLAGS) irq.c -o irq.o
	@$(GCC) $(GCCFLAGS) isr.c -o isr.o
	@$(GCC) $(GCCFLAGS) kbd.c -o kbd.o
	@$(GCC) $(GCCFLAGS) kernel.c -o kernel.o
	@$(GCC) $(GCCFLAGS) mboot.c -o mboot.o
	@$(GCC) $(GCCFLAGS) mmu.c -o mmu.o
	@$(GCC) $(GCCFLAGS) pit.c -o pit.o
	@$(GCC) $(GCCFLAGS) rtc.c -o rtc.o
	@$(GCC) $(GCCFLAGS) screen.c -o screen.o
	@$(GCC) $(GCCFLAGS) shell.c -o shell.o
	@$(GCC) $(GCCFLAGS) syscall.c -o syscall.o

clean:
	@rm -f *.o

