MOD:=$(MODULES_ARCH_PATH)/ide/ide.ko
OBJ:=$(MODULES_ARCH_PATH)/ide/main.o $(MODULES_ARCH_PATH)/ide/bus.o $(MODULES_ARCH_PATH)/ide/device.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
