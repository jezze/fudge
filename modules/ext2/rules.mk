MODULES+=modules/ext2/ext2.ko
MODULES_OBJECTS+=modules/ext2/main.o modules/ext2/protocol.o modules/ext2/filesystem.o

modules/ext2/ext2.ko: modules/ext2/main.o modules/ext2/protocol.o modules/ext2/filesystem.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
