MOD:=modules/ext2/ext2.ko
OBJ:=modules/ext2/main.o modules/ext2/protocol.o modules/ext2/filesystem.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
