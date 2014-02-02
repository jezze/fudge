MOD:=$(MODULES_ARCH_PATH)/platform/platform.ko
OBJ:=$(MODULES_ARCH_PATH)/platform/main.o $(MODULES_ARCH_PATH)/platform/bus.o $(MODULES_ARCH_PATH)/platform/device.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
