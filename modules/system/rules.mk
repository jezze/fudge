MOD:=$(MODULES_PATH)/system/system.ko
OBJ:=$(MODULES_PATH)/system/main.o $(MODULES_PATH)/system/backend.o $(MODULES_PATH)/system/protocol.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
