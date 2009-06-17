ASM=nasm
ASMFLAGS=-f elf
GCC=gcc
GCCFLAGS=-c -I./include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs
LD=ld
LDFLAGS=-T linker.ld

all: loader.o kernel.o string.o gdt.o tss.o idt.o isr.o isrs.o irq.o irqs.o pit.o screen.o keyboard.o heap.o paging.o fs.o initrd.o syscall.o console.o
	$(LD) $(LDFLAGS) obj/loader.o obj/kernel.o obj/string.o obj/gdt.o obj/tss.o obj/idt.o obj/isr.o obj/isrs.o obj/irq.o obj/irqs.o obj/pit.o obj/screen.o obj/keyboard.o obj/heap.o obj/paging.o obj/fs.o obj/initrd.o obj/syscall.o obj/console.o -o root/boot/kernel

cd:
	genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o fudge.iso root

qemu:
	qemu -cdrom fudge.iso

loader.o:
	$(ASM) $(ASMFLAGS) src/loader.s -o obj/loader.o

kernel.o:
	$(GCC) $(GCCFLAGS) src/kernel.c -o obj/kernel.o

string.o:
	$(GCC) $(GCCFLAGS) src/string.c -o obj/string.o

gdt.o:
	$(GCC) $(GCCFLAGS) src/gdt.c -o obj/gdt.o

tss.o:
	$(GCC) $(GCCFLAGS) src/tss.c -o obj/tss.o

idt.o:
	$(GCC) $(GCCFLAGS) src/idt.c -o obj/idt.o

isr.o:
	$(GCC) $(GCCFLAGS) src/isr.c -o obj/isr.o

isrs.o:
	$(ASM) $(ASMFLAGS) src/isrs.s -o obj/isrs.o

irq.o:
	$(GCC) $(GCCFLAGS) src/irq.c -o obj/irq.o

irqs.o:
	$(ASM) $(ASMFLAGS) src/irqs.s -o obj/irqs.o

pit.o:
	$(GCC) $(GCCFLAGS) src/pit.c -o obj/pit.o

screen.o:
	$(GCC) $(GCCFLAGS) src/screen.c -o obj/screen.o

keyboard.o:
	$(GCC) $(GCCFLAGS) src/keyboard.c -o obj/keyboard.o

heap.o:
	$(GCC) $(GCCFLAGS) src/heap.c -o obj/heap.o

paging.o:
	$(GCC) $(GCCFLAGS) src/paging.c -o obj/paging.o

fs.o:
	$(GCC) $(GCCFLAGS) src/fs.c -o obj/fs.o

initrd.o:
	$(GCC) $(GCCFLAGS) src/initrd.c -o obj/initrd.o

syscall.o:
	$(GCC) $(GCCFLAGS) src/syscall.c -o obj/syscall.o

console.o:
	$(GCC) $(GCCFLAGS) src/console.c -o obj/console.o

clean:
	rm jinx.iso
