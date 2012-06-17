MODULES+=modules/ext2/ext2.ko
CLEAN+=modules/ext2/main.o modules/ext2/driver.o modules/ext2/filesystem.o

modules/ext2/ext2.ko: lib/memory.o lib/string.o modules/ext2/main.o modules/ext2/driver.o modules/ext2/filesystem.o
	$(LD) $(LDFLAGS) -o $@ $^
