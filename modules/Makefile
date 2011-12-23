all-arm: ARCH=arm
all-arm: GCC=arm-none-eabi-gcc
all-arm: GCCFLAGS=-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs
all-arm: LD=arm-none-eabi-ld
all-arm: LDFLAGS=-r ../lib/memory.o ../lib/string.o -L/usr/lib/gcc/arm-none-eabi/4.6.1/ -lgcc
all-x86: ARCH=x86
all-x86: ASM=nasm
all-x86: ASMFLAGS=-f elf
all-x86: GCC=gcc
all-x86: GCCFLAGS=-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -m32
all-x86: LD=ld
all-x86: LDFLAGS=-r ../lib/memory.o ../lib/string.o -melf_i386

.PHONY: all clean modules

all: all-x86

all-arm:
	@${GCC} ${GCCFLAGS} log/main.c -o log/main.o
	@${LD} ${LDFLAGS} log/main.o -o log/log.ko

all-x86:
	@${GCC} ${GCCFLAGS} ata/bus.c -o ata/bus.o
	@${GCC} ${GCCFLAGS} ata/device.c -o ata/device.o
	@${GCC} ${GCCFLAGS} ata/main.c -o ata/main.o
	@${GCC} ${GCCFLAGS} bga/driver.c -o bga/driver.o
	@${GCC} ${GCCFLAGS} bga/main.c -o bga/main.o
	@${GCC} ${GCCFLAGS} ext2/ext2.c -o ext2/ext2.o
	@${GCC} ${GCCFLAGS} log/main.c -o log/main.o
	@${GCC} ${GCCFLAGS} mbr/mbr.c -o mbr/mbr.o
	@${GCC} ${GCCFLAGS} pci/pci.c -o pci/pci.o
	@${GCC} ${GCCFLAGS} pit/device.c -o pit/device.o
	@${GCC} ${GCCFLAGS} pit/main.c -o pit/main.o
	@${GCC} ${GCCFLAGS} ps2/kbd.c -o ps2/kbd.o
	@${GCC} ${GCCFLAGS} ps2/main.c -o ps2/main.o
	@${GCC} ${GCCFLAGS} ps2/mouse.c -o ps2/mouse.o
	@${GCC} ${GCCFLAGS} rtc/device.c -o rtc/device.o
	@${GCC} ${GCCFLAGS} rtc/main.c -o rtc/main.o
	@${GCC} ${GCCFLAGS} rtl8139/driver.c -o rtl8139/driver.o
	@${GCC} ${GCCFLAGS} rtl8139/main.c -o rtl8139/main.o
	@${GCC} ${GCCFLAGS} tty/device.c -o tty/device.o
	@${GCC} ${GCCFLAGS} tty/main.c -o tty/main.o
	@${GCC} ${GCCFLAGS} tty/view.c -o tty/view.o
	@${GCC} ${GCCFLAGS} uart/device.c -o uart/device.o
	@${GCC} ${GCCFLAGS} uart/main.c -o uart/main.o
	@${GCC} ${GCCFLAGS} vga/device.c -o vga/device.o
	@${GCC} ${GCCFLAGS} vga/main.c -o vga/main.o
	@${LD} ${LDFLAGS} ata/bus.o ata/device.o ata/main.o -o ata/ata.ko
	@${LD} ${LDFLAGS} bga/driver.o bga/main.o -o bga/bga.ko
	@${LD} ${LDFLAGS} ext2/ext2.o -o ext2/ext2.ko
	@${LD} ${LDFLAGS} log/main.o -o log/log.ko
	@${LD} ${LDFLAGS} mbr/mbr.o -o mbr/mbr.ko
	@${LD} ${LDFLAGS} pci/pci.o -o pci/pci.ko
	@${LD} ${LDFLAGS} pit/device.o pit/main.o -o pit/pit.ko
	@${LD} ${LDFLAGS} ps2/kbd.o ps2/main.o ps2/mouse.o -o ps2/ps2.ko
	@${LD} ${LDFLAGS} rtc/device.o rtc/main.o -o rtc/rtc.ko
	@${LD} ${LDFLAGS} rtl8139/driver.o rtl8139/main.o -o rtl8139/rtl8139.ko
	@${LD} ${LDFLAGS} tty/device.o tty/main.o tty/view.o -o tty/tty.ko
	@${LD} ${LDFLAGS} uart/device.o uart/main.o -o uart/uart.ko
	@${LD} ${LDFLAGS} vga/device.o vga/main.o -o vga/vga.ko

clean:
	@rm -f */*.o
	@rm -f */*.ko

