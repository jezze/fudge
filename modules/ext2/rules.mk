BUILD+=modules/ext2/ext2.ko
CLEAN+=$(OBJ_modules/ext2/ext2.ko) modules/ext2/ext2.ko

OBJ_modules/ext2/ext2.ko:=modules/ext2/main.o modules/ext2/driver.o modules/ext2/filesystem.o

modules/ext2/ext2.ko: GCCFLAGS+=-Iinclude
modules/ext2/ext2.ko: LDFLAGS+=-Tmodules/linker.ld -r lib/memory.o lib/string.o
modules/ext2/ext2.ko: lib $(OBJ_modules/ext2/ext2.ko)
	$(LD) $(LDFLAGS) -o $@ $(OBJ_modules/ext2/ext2.ko)

