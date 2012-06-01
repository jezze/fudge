include modules/arch/${ARCH}/rules.mk

modules: GCCFLAGS+=-Iinclude
modules: LDFLAGS+=-Tmodules/linker.ld -r lib/memory.o lib/string.o

modules: modules-arch
	@${GCC} ${GCCFLAGS} modules/ext2/driver.c -o modules/ext2/driver.o
	@${GCC} ${GCCFLAGS} modules/ext2/filesystem.c -o modules/ext2/filesystem.o
	@${GCC} ${GCCFLAGS} modules/ext2/main.c -o modules/ext2/main.o
	@${GCC} ${GCCFLAGS} modules/log/driver.c -o modules/log/driver.o
	@${GCC} ${GCCFLAGS} modules/log/main.c -o modules/log/main.o
	@${GCC} ${GCCFLAGS} modules/mbr/device.c -o modules/mbr/device.o
	@${GCC} ${GCCFLAGS} modules/mbr/driver.c -o modules/mbr/driver.o
	@${GCC} ${GCCFLAGS} modules/mbr/main.c -o modules/mbr/main.o
	@${GCC} ${GCCFLAGS} modules/nodefs/driver.c -o modules/nodefs/driver.o
	@${GCC} ${GCCFLAGS} modules/nodefs/main.c -o modules/nodefs/main.o
	@${GCC} ${GCCFLAGS} modules/tty/driver.c -o modules/tty/driver.o
	@${GCC} ${GCCFLAGS} modules/tty/main.c -o modules/tty/main.o
	@mkdir -p image/mod
	@${LD} ${LDFLAGS} modules/ext2/driver.o modules/ext2/filesystem.o modules/ext2/main.o -o image/mod/ext2.ko
	@${LD} ${LDFLAGS} modules/log/driver.o modules/log/main.o -o image/mod/log.ko
	@${LD} ${LDFLAGS} modules/mbr/device.o modules/mbr/driver.o modules/mbr/main.o -o image/mod/mbr.ko
	@${LD} ${LDFLAGS} modules/nodefs/driver.o modules/nodefs/main.o -o image/mod/nodefs.ko
	@${LD} ${LDFLAGS} modules/tty/driver.o modules/tty/main.o -o image/mod/tty.ko

modules-clean: modules-arch-clean
	@rm -f modules/*/*.o
	@rm -f image/mod/ext2.ko
	@rm -f image/mod/log.ko
	@rm -f image/mod/mbr.ko
	@rm -f image/mod/nodefs.ko
	@rm -f image/mod/tty.ko
	@rm -rf image/mod

