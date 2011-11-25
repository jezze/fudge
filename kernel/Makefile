all-arm: ARCH=arm
all-arm: GCC=arm-elf-gcc
all-arm: GCCFLAGS=-c -O2 -I../include -Wall -Wextra -nostdlib -nostartfiles -ffreestanding -std=gnu99
all-arm: LD=arm-elf-ld
all-arm: LDFLAGS=-T${DIR_SOURCE_ARCH}/linker.ld -L/usr/lib/gcc/arm-elf/4.6.0
all-x86: ARCH=x86
all-x86: GCC=gcc
all-x86: GCCFLAGS=-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -m32
all-x86: LD=ld
all-x86: LDFLAGS=-T${DIR_SOURCE_ARCH}/linker.ld -melf_i386

DIR_SOURCE_ARCH=arch/${ARCH}
DIR_SOURCE_LIB=../lib

.PHONY: all clean kernel

all: all-x86
all-arm: kernel arch-arm
all-x86: kernel arch-x86

kernel:
	@${GCC} ${GCCFLAGS} elf.c -o elf.o
	@${GCC} ${GCCFLAGS} error.c -o error.o
	@${GCC} ${GCCFLAGS} event.c -o event.o
	@${GCC} ${GCCFLAGS} initrd.c -o initrd.o
	@${GCC} ${GCCFLAGS} kernel.c -o kernel.o
	@${GCC} ${GCCFLAGS} log.c -o log.o
	@${GCC} ${GCCFLAGS} mmu.c -o mmu.o
	@${GCC} ${GCCFLAGS} modules.c -o modules.o
	@${GCC} ${GCCFLAGS} runtime.c -o runtime.o
	@${GCC} ${GCCFLAGS} symbol.c -o symbol.o
	@${GCC} ${GCCFLAGS} syscall.c -o syscall.o
	@${GCC} ${GCCFLAGS} vfs.c -o vfs.o

arch-arm:
	@make -C ${DIR_SOURCE_ARCH}/
	@${LD} elf.o error.o event.o initrd.o kernel.o log.o mmu.o modules.o runtime.o symbol.o syscall.o vfs.o \
		${DIR_SOURCE_ARCH}/arch.o \
		${DIR_SOURCE_ARCH}/call.o \
		${DIR_SOURCE_ARCH}/init.o \
		${DIR_SOURCE_LIB}/memory.o \
		${DIR_SOURCE_LIB}/file.o \
		${DIR_SOURCE_LIB}/string.o \
		-o fudge ${LDFLAGS} 

arch-x86:
	@make -C ${DIR_SOURCE_ARCH}/
	@${LD} ${LDFLAGS} elf.o error.o event.o initrd.o kernel.o log.o mmu.o modules.o runtime.o symbol.o syscall.o vfs.o \
		${DIR_SOURCE_ARCH}/arch.o \
		${DIR_SOURCE_ARCH}/calls.o \
		${DIR_SOURCE_ARCH}/cpu.o \
		${DIR_SOURCE_ARCH}/fpu.o \
		${DIR_SOURCE_ARCH}/gdt.o \
		${DIR_SOURCE_ARCH}/idt.o \
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
		${DIR_SOURCE_LIB}/memory.o \
		${DIR_SOURCE_LIB}/file.o \
		${DIR_SOURCE_LIB}/string.o \
		-o fudge

clean:
	@make -C arch/arm/ clean
	@make -C arch/x86/ clean
	@rm -f fudge
	@rm -f *.o

