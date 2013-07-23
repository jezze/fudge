INIT_BIN:=packages/init/init
INIT_OBJ:=packages/init/init.o

INITFS_BIN:=packages/init/initfs
INITFS_OBJ:=packages/init/initfs.o

$(INIT_BIN): $(INIT_OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

$(INITFS_BIN): $(INITFS_OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(INIT_BIN) $(INITFS_BIN)
PACKAGES_OBJECTS+=$(INIT_OBJ) $(INITFS_OBJ)
