INIT_BIN:=packages/init/init
INIT_OBJ:=packages/init/init.o

INITMOD_BIN:=packages/init/initmod
INITMOD_OBJ:=packages/init/initmod.o

INITFS_BIN:=packages/init/initfs
INITFS_OBJ:=packages/init/initfs.o

$(INIT_BIN): $(INIT_OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

$(INITMOD_BIN): $(INITMOD_OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

$(INITFS_BIN): $(INITFS_OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(INIT_BIN) $(INITMOD_BIN) $(INITFS_BIN)
PACKAGES_OBJECTS+=$(INIT_OBJ) $(INITMOD_OBJ) $(INITFS_OBJ)
