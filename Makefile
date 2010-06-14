ASM=nasm
ASMFLAGS=-f elf
GCC=gcc
GCCFLAGS=-c -O2 -I./include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs
LD=ld
LDFLAGS=-T linker.ld
ISO=genisoimage
ISOFLAGS=-R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table 
MKINITRD=./tools/mkinitrd

.PHONY: all kernel library cd innitrd clean

all: kernel cd

cd:
	@echo "Creating iso..."
	@$(ISO) $(ISOFLAGS) -o fudge.iso root

clean:
	@echo "Cleaning..."
	@rm -f lib/*.o
	@rm -f kernel/*.o
	@rm -f tools/mkinitrd
	@rm -f root/boot/kernel
	@rm -f root/boot/initrd
	@rm -f fudge.iso

initrd:
	@echo "Creating ramdisk..."
	@$(GCC) -O2 tools/mkinitrd.c -o tools/mkinitrd
	$(MKINITRD) ramdisk/about.txt about.txt ramdisk/help.txt help.txt ramdisk/hello.s hello.s ramdisk/hello hello ramdisk/hello2.c hello2.c ramdisk/hello2 hello2

kernel: library
	@echo "Building kernel..."
	@$(GCC) $(GCCFLAGS) kernel/assert.c -o kernel/assert.o
	@$(GCC) $(GCCFLAGS) kernel/kernel.c -o kernel/kernel.o
	@$(ASM) $(ASMFLAGS) kernel/loader.s -o kernel/loader.o
	@$(GCC) $(GCCFLAGS) kernel/gdt.c -o kernel/gdt.o
	@$(ASM) $(ASMFLAGS) kernel/gdts.s -o kernel/gdts.o
	@$(GCC) $(GCCFLAGS) kernel/idt.c -o kernel/idt.o
	@$(ASM) $(ASMFLAGS) kernel/idts.s -o kernel/idts.o
	@$(GCC) $(GCCFLAGS) kernel/isr.c -o kernel/isr.o
	@$(ASM) $(ASMFLAGS) kernel/isrs.s -o kernel/isrs.o
	@$(GCC) $(GCCFLAGS) kernel/irq.c -o kernel/irq.o
	@$(ASM) $(ASMFLAGS) kernel/irqs.s -o kernel/irqs.o
	@$(GCC) $(GCCFLAGS) kernel/pit.c -o kernel/pit.o
	@$(GCC) $(GCCFLAGS) kernel/screen.c -o kernel/screen.o
	@$(GCC) $(GCCFLAGS) kernel/kbd.c -o kernel/kbd.o
	@$(GCC) $(GCCFLAGS) kernel/heap.c -o kernel/heap.o
	@$(GCC) $(GCCFLAGS) kernel/tlb.c -o kernel/tlb.o
	@$(ASM) $(ASMFLAGS) kernel/tlbs.s -o kernel/tlbs.o
	@$(GCC) $(GCCFLAGS) kernel/vfs.c -o kernel/vfs.o
	@$(GCC) $(GCCFLAGS) kernel/initrd.c -o kernel/initrd.o
	@$(GCC) $(GCCFLAGS) kernel/syscall.c -o kernel/syscall.o
	@$(GCC) $(GCCFLAGS) kernel/task.c -o kernel/task.o
	@$(ASM) $(ASMFLAGS) kernel/vbes.s -o kernel/vbes.o
	@$(GCC) $(GCCFLAGS) kernel/vbe.c -o kernel/vbe.o
	@$(GCC) $(GCCFLAGS) kernel/shell.c -o kernel/shell.o
	@$(LD) $(LDFLAGS) \
    kernel/loader.o \
    kernel/kernel.o \
    kernel/assert.o \
    lib/memory.o \
    lib/string.o \
    lib/cbuffer.o \
    lib/stack.o \
    kernel/gdt.o \
    kernel/gdts.o \
    kernel/idt.o \
    kernel/idts.o \
    kernel/isr.o \
    kernel/isrs.o \
    kernel/irq.o \
    kernel/irqs.o \
    kernel/pit.o \
    kernel/screen.o \
    kernel/kbd.o \
    kernel/heap.o \
    kernel/tlb.o \
    kernel/tlbs.o \
    kernel/vfs.o \
    kernel/initrd.o \
    kernel/syscall.o \
    kernel/task.o \
    kernel/vbes.o \
    lib/io.o \
    kernel/vbe.o \
    kernel/shell.o \
    -o root/boot/kernel

library:
	@echo "Building library..."
	@$(GCC) $(GCCFLAGS) lib/stack.c -o lib/stack.o
	@$(GCC) $(GCCFLAGS) lib/cbuffer.c -o lib/cbuffer.o
	@$(GCC) $(GCCFLAGS) lib/string.c -o lib/string.o
	@$(GCC) $(GCCFLAGS) lib/memory.c -o lib/memory.o
	@$(GCC) $(GCCFLAGS) lib/io.c -o lib/io.o

