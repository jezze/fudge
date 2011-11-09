ASM=nasm
ASMFLAGS=-f elf
GCC=gcc
GCCFLAGS=-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -m32
LD=ld
LDFLAGS=-r ../lib/file.o ../lib/memory.o ../lib/string.o ../kernel/arch/x86/calls.o

.PHONY: all clean modules

all: modules

modules:
	@${GCC} ${GCCFLAGS} ata/ata.c -o ata/ata.o
	@${GCC} ${GCCFLAGS} elf/elf.c -o elf/elf.o
	@${GCC} ${GCCFLAGS} kbd/kbd.c -o kbd/kbd.o
	@${GCC} ${GCCFLAGS} pci/pci.c -o pci/pci.o
	@${GCC} ${GCCFLAGS} pit/pit.c -o pit/pit.o
	@${GCC} ${GCCFLAGS} rtc/rtc.c -o rtc/rtc.o
	@${GCC} ${GCCFLAGS} rtl8139/rtl8139.c -o rtl8139/rtl8139.o
	@${GCC} ${GCCFLAGS} serial/serial.c -o serial/serial.o
	@${GCC} ${GCCFLAGS} stream/stream.c -o stream/stream.o
	@${GCC} ${GCCFLAGS} tty/tty.c -o tty/tty.o
	@${GCC} ${GCCFLAGS} vga/vga.c -o vga/vga.o
	@${LD} ${LDFLAGS} ata/ata.o -o ata/ata.ko
	@${LD} ${LDFLAGS} elf/elf.o -o elf/elf.ko
	@${LD} ${LDFLAGS} kbd/kbd.o -o kbd/kbd.ko
	@${LD} ${LDFLAGS} pci/pci.o -o pci/pci.ko
	@${LD} ${LDFLAGS} pit/pit.o stream/stream.o -o pit/pit.ko
	@${LD} ${LDFLAGS} rtc/rtc.o stream/stream.o -o rtc/rtc.ko
	@${LD} ${LDFLAGS} pci/pci.o rtl8139/rtl8139.o -o rtl8139/rtl8139.ko
	@${LD} ${LDFLAGS} serial/serial.o -o serial/serial.ko
	@${LD} ${LDFLAGS} stream/stream.o -o stream/stream.ko
	@${LD} ${LDFLAGS} stream/stream.o tty/tty.o -o tty/tty.ko
	@${LD} ${LDFLAGS} vga/vga.o -o vga/vga.ko

clean:
	@rm -f */*.o
	@rm -f */*.ko

