vfs: GCCFLAGS+=-Iinclude

vfs:
	@${GCC} ${GCCFLAGS} kernel/vfs/sys.c -o kernel/vfs/sys.o
	@${GCC} ${GCCFLAGS} kernel/vfs/ramdisk.c -o kernel/vfs/ramdisk.o

vfs-clean:
	@rm -f kernel/vfs/*.o

