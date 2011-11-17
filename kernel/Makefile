ARCH=x86
DIR_SOURCE_ARCH=arch/${ARCH}
DIR_SOURCE_LIB=../lib

GCC=gcc
GCCFLAGS=-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -m32
LD=ld
LDFLAGS=-T${DIR_SOURCE_ARCH}/linker.ld -melf_i386

.PHONY: all clean kernel

all: kernel

kernel:
	@make -C ${DIR_SOURCE_ARCH}/
	@${GCC} ${GCCFLAGS} elf.c -o elf.o
	@${GCC} ${GCCFLAGS} error.c -o error.o
	@${GCC} ${GCCFLAGS} event.c -o event.o
	@${GCC} ${GCCFLAGS} initrd.c -o initrd.o
	@${GCC} ${GCCFLAGS} kernel.c -o kernel.o
	@${GCC} ${GCCFLAGS} log.c -o log.o
	@${GCC} ${GCCFLAGS} modules.c -o modules.o
	@${GCC} ${GCCFLAGS} runtime.c -o runtime.o
	@${GCC} ${GCCFLAGS} syscall.c -o syscall.o
	@${GCC} ${GCCFLAGS} vfs.c -o vfs.o
	@${LD} ${LDFLAGS} elf.o error.o event.o initrd.o kernel.o log.o modules.o runtime.o syscall.o vfs.o \
		${DIR_SOURCE_ARCH}/arch.o \
		${DIR_SOURCE_ARCH}/calls.o \
		${DIR_SOURCE_ARCH}/cpu.o \
		${DIR_SOURCE_ARCH}/fpu.o \
		${DIR_SOURCE_ARCH}/fpus.o \
		${DIR_SOURCE_ARCH}/gdt.o \
		${DIR_SOURCE_ARCH}/gdts.o \
		${DIR_SOURCE_ARCH}/idt.o \
		${DIR_SOURCE_ARCH}/idts.o \
		${DIR_SOURCE_ARCH}/init.o \
		${DIR_SOURCE_ARCH}/ios.o \
		${DIR_SOURCE_ARCH}/irqs.o \
		${DIR_SOURCE_ARCH}/isrs.o \
		${DIR_SOURCE_ARCH}/irq.o \
		${DIR_SOURCE_ARCH}/isr.o \
		${DIR_SOURCE_ARCH}/mmu.o \
		${DIR_SOURCE_ARCH}/mboot.o \
		${DIR_SOURCE_ARCH}/syscall.o \
		${DIR_SOURCE_ARCH}/tss.o \
		${DIR_SOURCE_ARCH}/tsss.o \
		${DIR_SOURCE_LIB}/memory.o \
		${DIR_SOURCE_LIB}/file.o \
		${DIR_SOURCE_LIB}/string.o \
		-o fudge

clean:
	@make -C arch/arm/ clean
	@make -C arch/x86/ clean
	@rm -f fudge
	@rm -f *.o

