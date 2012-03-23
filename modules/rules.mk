include modules/arch/${ARCH}/rules.mk

modules: GCCFLAGS+=-Iinclude
modules: LDFLAGS+=-Tmodules/linker.ld -r lib/memory.o lib/string.o

modules: modules-arch
	@${GCC} ${GCCFLAGS} modules/ext2/driver.c -o modules/ext2/driver.o
	@${GCC} ${GCCFLAGS} modules/ext2/filesystem.c -o modules/ext2/filesystem.o
	@${GCC} ${GCCFLAGS} modules/ext2/main.c -o modules/ext2/main.o
	@${GCC} ${GCCFLAGS} modules/mbr/driver.c -o modules/mbr/driver.o
	@${GCC} ${GCCFLAGS} modules/mbr/main.c -o modules/mbr/main.o
	@${GCC} ${GCCFLAGS} modules/nodefs/driver.c -o modules/nodefs/driver.o
	@${GCC} ${GCCFLAGS} modules/nodefs/main.c -o modules/nodefs/main.o
	@${GCC} ${GCCFLAGS} modules/tty/driver.c -o modules/tty/driver.o
	@${GCC} ${GCCFLAGS} modules/tty/main.c -o modules/tty/main.o
	@${LD} ${LDFLAGS} modules/ext2/driver.o modules/ext2/filesystem.o modules/ext2/main.o -o modules/ext2/ext2.ko
	@${LD} ${LDFLAGS} modules/mbr/driver.o modules/mbr/main.o -o modules/mbr/mbr.ko
	@${LD} ${LDFLAGS} modules/nodefs/driver.o modules/nodefs/main.o -o modules/nodefs/nodefs.ko
	@${LD} ${LDFLAGS} modules/tty/driver.o modules/tty/main.o -o modules/tty/tty.ko

modules-clean: modules-arch-clean
	@rm -f modules/*/*.o
	@rm -f modules/*/*.ko

