include ../rules.mk
include ../${ARCH}.mk

GCCFLAGS+=-I../include
LDFLAGS+=-r ../lib/memory.o ../lib/string.o

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
	@${GCC} ${GCCFLAGS} ext2/driver.c -o ext2/driver.o
	@${GCC} ${GCCFLAGS} ext2/main.c -o ext2/main.o
	@${GCC} ${GCCFLAGS} log/main.c -o log/main.o
	@${GCC} ${GCCFLAGS} mbr/driver.c -o mbr/driver.o
	@${GCC} ${GCCFLAGS} mbr/main.c -o mbr/main.o
	@${GCC} ${GCCFLAGS} pci/bus.c -o pci/bus.o
	@${GCC} ${GCCFLAGS} pci/device.c -o pci/device.o
	@${GCC} ${GCCFLAGS} pci/main.c -o pci/main.o
	@${GCC} ${GCCFLAGS} pit/device.c -o pit/device.o
	@${GCC} ${GCCFLAGS} pit/main.c -o pit/main.o
	@${GCC} ${GCCFLAGS} ps2/kbd_device.c -o ps2/kbd_device.o
	@${GCC} ${GCCFLAGS} ps2/kbd_driver.c -o ps2/kbd_driver.o
	@${GCC} ${GCCFLAGS} ps2/main.c -o ps2/main.o
	@${GCC} ${GCCFLAGS} ps2/mouse_device.c -o ps2/mouse_device.o
	@${GCC} ${GCCFLAGS} ps2/mouse_driver.c -o ps2/mouse_driver.o
	@${GCC} ${GCCFLAGS} rtc/device.c -o rtc/device.o
	@${GCC} ${GCCFLAGS} rtc/main.c -o rtc/main.o
	@${GCC} ${GCCFLAGS} rtl8139/driver.c -o rtl8139/driver.o
	@${GCC} ${GCCFLAGS} rtl8139/main.c -o rtl8139/main.o
	@${GCC} ${GCCFLAGS} tty/driver.c -o tty/driver.o
	@${GCC} ${GCCFLAGS} tty/main.c -o tty/main.o
	@${GCC} ${GCCFLAGS} tty/view.c -o tty/view.o
	@${GCC} ${GCCFLAGS} uart/device.c -o uart/device.o
	@${GCC} ${GCCFLAGS} uart/driver.c -o uart/driver.o
	@${GCC} ${GCCFLAGS} uart/main.c -o uart/main.o
	@${GCC} ${GCCFLAGS} vga/device.c -o vga/device.o
	@${GCC} ${GCCFLAGS} vga/main.c -o vga/main.o
	@${LD} ${LDFLAGS} ata/bus.o ata/device.o ata/main.o -o ata/ata.ko
	@${LD} ${LDFLAGS} bga/driver.o bga/main.o -o bga/bga.ko
	@${LD} ${LDFLAGS} ext2/driver.o ext2/main.o -o ext2/ext2.ko
	@${LD} ${LDFLAGS} log/main.o -o log/log.ko
	@${LD} ${LDFLAGS} mbr/driver.o mbr/main.o -o mbr/mbr.ko
	@${LD} ${LDFLAGS} pci/bus.o pci/device.o pci/main.o -o pci/pci.ko
	@${LD} ${LDFLAGS} pit/device.o pit/main.o -o pit/pit.ko
	@${LD} ${LDFLAGS} ps2/kbd_device.o ps2/kbd_driver.o ps2/main.o ps2/mouse_device.o ps2/mouse_driver.o -o ps2/ps2.ko
	@${LD} ${LDFLAGS} rtc/device.o rtc/main.o -o rtc/rtc.ko
	@${LD} ${LDFLAGS} rtl8139/driver.o rtl8139/main.o -o rtl8139/rtl8139.ko
	@${LD} ${LDFLAGS} tty/driver.o tty/main.o tty/view.o -o tty/tty.ko
	@${LD} ${LDFLAGS} uart/driver.o uart/device.o uart/main.o -o uart/uart.ko
	@${LD} ${LDFLAGS} vga/device.o vga/main.o -o vga/vga.ko

clean:
	@rm -f */*.o
	@rm -f */*.ko

