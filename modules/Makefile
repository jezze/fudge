all-arm: ARCH=arm
all-arm: GCC=arm-none-eabi-gcc
all-arm: GCCFLAGS=-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs
all-arm: LD=arm-none-eabi-ld
all-arm: LDFLAGS=-r ../lib/file.o ../lib/memory.o ../lib/string.o ../kernel/arch/arm/call.o -L/usr/lib/gcc/arm-none-eabi/4.6.1/ -lgcc
all-x86: ARCH=x86
all-x86: ASM=nasm
all-x86: ASMFLAGS=-f elf
all-x86: GCC=gcc
all-x86: GCCFLAGS=-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -m32
all-x86: LD=ld
all-x86: LDFLAGS=-r ../lib/file.o ../lib/memory.o ../lib/string.o ../kernel/arch/x86/calls.o -melf_i386

.PHONY: all clean modules

all: all-x86

all-arm:
	@${GCC} ${GCCFLAGS} stream/stream.c -o stream/stream.o
	@${LD} ${LDFLAGS} stream/stream.o -o stream/stream.ko

all-x86:
	@${GCC} ${GCCFLAGS} ata/ata.c -o ata/ata.o
	@${GCC} ${GCCFLAGS} pci/pci.c -o pci/pci.o
	@${GCC} ${GCCFLAGS} pit/pit.c -o pit/pit.o
	@${GCC} ${GCCFLAGS} ps2/kbd.c -o ps2/kbd.o
	@${GCC} ${GCCFLAGS} ps2/mouse.c -o ps2/mouse.o
	@${GCC} ${GCCFLAGS} rtc/rtc.c -o rtc/rtc.o
	@${GCC} ${GCCFLAGS} rtl8139/rtl8139.c -o rtl8139/rtl8139.o
	@${GCC} ${GCCFLAGS} stream/stream.c -o stream/stream.o
	@${GCC} ${GCCFLAGS} tty/tty.c -o tty/tty.o
	@${GCC} ${GCCFLAGS} uart/uart.c -o uart/uart.o
	@${GCC} ${GCCFLAGS} vga/vga.c -o vga/vga.o
	@${LD} ${LDFLAGS} ata/ata.o -o ata/ata.ko
	@${LD} ${LDFLAGS} pci/pci.o -o pci/pci.ko
	@${LD} ${LDFLAGS} pit/pit.o stream/stream.o -o pit/pit.ko
	@${LD} ${LDFLAGS} ps2/kbd.o -o ps2/ps2_kbd.ko
	@${LD} ${LDFLAGS} ps2/mouse.o -o ps2/ps2_mouse.ko
	@${LD} ${LDFLAGS} rtc/rtc.o stream/stream.o -o rtc/rtc.ko
	@${LD} ${LDFLAGS} rtl8139/rtl8139.o stream/stream.o -o rtl8139/rtl8139.ko
	@${LD} ${LDFLAGS} stream/stream.o -o stream/stream.ko
	@${LD} ${LDFLAGS} stream/stream.o tty/tty.o -o tty/tty.ko
	@${LD} ${LDFLAGS} uart/uart.o -o uart/uart.ko
	@${LD} ${LDFLAGS} vga/vga.o -o vga/vga.ko

clean:
	@rm -f */*.o
	@rm -f */*.ko

