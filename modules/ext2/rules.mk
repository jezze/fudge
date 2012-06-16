BUILD+=modules/ext2/ext2.ko
CLEAN+=$(OBJ_modules/ext2/ext2.ko) modules/ext2/ext2.ko

OBJ_modules/ext2/ext2.ko:=modules/ext2/main.o modules/ext2/driver.o modules/ext2/filesystem.o

modules/ext2/ext2.ko: lib/memory.o lib/string.o $(OBJ_modules/ext2/ext2.ko)
	$(LD) $(LDFLAGS) -o $@ $^
