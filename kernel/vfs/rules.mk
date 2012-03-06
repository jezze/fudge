vfs: GCCFLAGS+=-Iinclude

vfs:
	@${GCC} ${GCCFLAGS} kernel/vfs/ramdisk.c -o kernel/vfs/ramdisk.o
	@${GCC} ${GCCFLAGS} kernel/vfs/root.c -o kernel/vfs/root.o
	@${GCC} ${GCCFLAGS} kernel/vfs/sys.c -o kernel/vfs/sys.o

vfs-clean:
	@rm -f kernel/vfs/*.o

