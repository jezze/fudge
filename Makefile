ASM=nasm
ASMFLAGS=-f elf
GCC=gcc
GCCFLAGS=-c -O2 -I./include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs
LD=ld
LDFLAGS=-T linker.ld

all: loader kernel mem string gdt idt isr irq pit screen kbd heap paging fs initrd syscall task shell
	$(LD) $(LDFLAGS) obj/loader.o obj/kernel.o obj/mem.o obj/string.o obj/gdt.o obj/gdts.o obj/idt.o obj/idts.o obj/isr.o obj/isrs.o obj/irq.o obj/irqs.o obj/pit.o obj/screen.o obj/kbd.o obj/heap.o obj/paging.o obj/fs.o obj/initrd.o obj/syscall.o obj/task.o obj/shell.o -o root/boot/kernel

cd:
	genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o fudge.iso root

loader:
	$(ASM) $(ASMFLAGS) src/loader.s -o obj/loader.o

kernel:
	$(GCC) $(GCCFLAGS) src/kernel.c -o obj/kernel.o

mem:
	$(GCC) $(GCCFLAGS) src/mem.c -o obj/mem.o

string:
	$(GCC) $(GCCFLAGS) src/string.c -o obj/string.o

gdt:
	$(GCC) $(GCCFLAGS) src/gdt.c -o obj/gdt.o
	$(ASM) $(ASMFLAGS) src/gdts.s -o obj/gdts.o

idt:
	$(GCC) $(GCCFLAGS) src/idt.c -o obj/idt.o
	$(ASM) $(ASMFLAGS) src/idts.s -o obj/idts.o

isr:
	$(GCC) $(GCCFLAGS) src/isr.c -o obj/isr.o
	$(ASM) $(ASMFLAGS) src/isrs.s -o obj/isrs.o

irq:
	$(GCC) $(GCCFLAGS) src/irq.c -o obj/irq.o
	$(ASM) $(ASMFLAGS) src/irqs.s -o obj/irqs.o

pit:
	$(GCC) $(GCCFLAGS) src/pit.c -o obj/pit.o

screen:
	$(GCC) $(GCCFLAGS) src/screen.c -o obj/screen.o

kbd:
	$(GCC) $(GCCFLAGS) src/kbd.c -o obj/kbd.o

heap:
	$(GCC) $(GCCFLAGS) src/heap.c -o obj/heap.o

paging:
	$(GCC) $(GCCFLAGS) src/paging.c -o obj/paging.o

fs:
	$(GCC) $(GCCFLAGS) src/fs.c -o obj/fs.o

initrd:
	$(GCC) $(GCCFLAGS) src/initrd.c -o obj/initrd.o

syscall:
	$(GCC) $(GCCFLAGS) src/syscall.c -o obj/syscall.o

task:
	$(GCC) $(GCCFLAGS) src/task.c -o obj/task.o

shell:
	$(GCC) $(GCCFLAGS) src/shell.c -o obj/shell.o

clean:
	rm obj/*
	rm root/boot/kernel
	rm fudge.iso
