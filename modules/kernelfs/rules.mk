MOD:=$(MODULES_PATH)/kernelfs/kernelfs.ko
OBJ:=$(MODULES_PATH)/kernelfs/main.o $(MODULES_PATH)/kernelfs/backend.o $(MODULES_PATH)/kernelfs/protocol.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
