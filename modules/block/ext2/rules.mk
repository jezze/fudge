MOD:=modules/block/ext2/ext2.ko
OBJ:=modules/block/ext2/main.o modules/block/ext2/protocol.o modules/block/ext2/filesystem.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
