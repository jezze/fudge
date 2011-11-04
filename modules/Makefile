ASM=nasm
ASMFLAGS=-f elf
GCC=gcc
GCCFLAGS=-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -m32

.PHONY: all clean modules

all: modules

modules:
	@${ASM} ${ASMFLAGS} io/ios.s -o io/ios.o
	@${GCC} ${GCCFLAGS} test/test.c -o test/test.o
	@${GCC} ${GCCFLAGS} ata/ata.c -o ata/ata.o
	@${GCC} ${GCCFLAGS} elf/elf.c -o elf/elf.o
	@${GCC} ${GCCFLAGS} io/io.c -o io/io.o
	@${GCC} ${GCCFLAGS} kbd/kbd.c -o kbd/kbd.o
	@${GCC} ${GCCFLAGS} pci/pci.c -o pci/pci.o
	@${GCC} ${GCCFLAGS} pit/pit.c -o pit/pit.o
	@${GCC} ${GCCFLAGS} rtc/rtc.c -o rtc/rtc.o
	@${GCC} ${GCCFLAGS} rtl8139/rtl8139.c -o rtl8139/rtl8139.o
	@${GCC} ${GCCFLAGS} serial/serial.c -o serial/serial.o
	@${GCC} ${GCCFLAGS} stream/stream.c -o stream/stream.o
	@${GCC} ${GCCFLAGS} tty/tty.c -o tty/tty.o
	@${GCC} ${GCCFLAGS} vga/vga.c -o vga/vga.o

clean:
	@rm -f */*.o

