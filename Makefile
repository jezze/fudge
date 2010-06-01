ASM=nasm
ASMFLAGS=-f elf
GCC=gcc
GCCFLAGS=-c -O2 -I./include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs
LD=ld
LDFLAGS=-T linker.ld
ISO=genisoimage
ISOFLAGS=-R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table 
MKINITRD=./mkinitrd

all: kernel cd

cd:
	$(ISO) $(ISOFLAGS) -o fudge.iso root

clean:
	rm -f obj/*
	rm -f root/boot/kernel
	rm -f root/boot/initrd
	rm -f mkinitrd
	rm -f fudge.iso

initrd:
	$(MKINITRD) root/initrd/about.txt about.txt root/initrd/help.s help.s root/initrd/help help

kernel: library
	$(GCC) $(GCCFLAGS) src/kernel/assert.c -o obj/assert.o
	$(GCC) $(GCCFLAGS) src/kernel/kernel.c -o obj/kernel.o
	$(ASM) $(ASMFLAGS) src/kernel/loader.s -o obj/loader.o
	$(GCC) $(GCCFLAGS) src/kernel/gdt.c -o obj/gdt.o
	$(ASM) $(ASMFLAGS) src/kernel/gdts.s -o obj/gdts.o
	$(GCC) $(GCCFLAGS) src/kernel/idt.c -o obj/idt.o
	$(ASM) $(ASMFLAGS) src/kernel/idts.s -o obj/idts.o
	$(GCC) $(GCCFLAGS) src/kernel/isr.c -o obj/isr.o
	$(ASM) $(ASMFLAGS) src/kernel/isrs.s -o obj/isrs.o
	$(GCC) $(GCCFLAGS) src/kernel/irq.c -o obj/irq.o
	$(ASM) $(ASMFLAGS) src/kernel/irqs.s -o obj/irqs.o
	$(GCC) $(GCCFLAGS) src/kernel/pit.c -o obj/pit.o
	$(GCC) $(GCCFLAGS) src/kernel/screen.c -o obj/screen.o
	$(GCC) $(GCCFLAGS) src/kernel/kbd.c -o obj/kbd.o
	$(GCC) $(GCCFLAGS) src/kernel/heap.c -o obj/heap.o
	$(GCC) $(GCCFLAGS) src/kernel/paging.c -o obj/paging.o
	$(GCC) $(GCCFLAGS) src/kernel/vfs.c -o obj/vfs.o
	$(GCC) $(GCCFLAGS) src/kernel/initrd.c -o obj/initrd.o
	$(GCC) $(GCCFLAGS) src/kernel/syscall.c -o obj/syscall.o
	$(GCC) $(GCCFLAGS) src/kernel/task.c -o obj/task.o
	$(GCC) $(GCCFLAGS) src/kernel/shell.c -o obj/shell.o
	$(LD) $(LDFLAGS) obj/loader.o obj/kernel.o obj/assert.o obj/mem.o obj/string.o obj/cbuffer.o obj/stack.o obj/gdt.o obj/gdts.o obj/idt.o obj/idts.o obj/isr.o obj/isrs.o obj/irq.o obj/irqs.o obj/pit.o obj/screen.o obj/kbd.o obj/heap.o obj/paging.o obj/vfs.o obj/initrd.o obj/syscall.o obj/task.o obj/shell.o -o root/boot/kernel

library:
	$(GCC) $(GCCFLAGS) src/lib/stack.c -o obj/stack.o
	$(GCC) $(GCCFLAGS) src/lib/cbuffer.c -o obj/cbuffer.o
	$(GCC) $(GCCFLAGS) src/lib/string.c -o obj/string.o
	$(GCC) $(GCCFLAGS) src/lib/mem.c -o obj/mem.o

tools:
	$(GCC) -O2 src/initrd/mkinitrd.c -o mkinitrd

