MODULES+=modules/src/ext2/ext2.ko
CLEAN+=modules/src/ext2/main.o modules/src/ext2/driver.o modules/src/ext2/filesystem.o

modules/src/ext2/ext2.ko: lib/memory.o lib/string.o modules/src/ext2/main.o modules/src/ext2/driver.o modules/src/ext2/filesystem.o
	$(LD) $(LDFLAGS) -o $@ $^
