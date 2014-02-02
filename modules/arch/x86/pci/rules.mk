MOD:=$(MODULES_ARCH_PATH)/pci/pci.ko
OBJ:=$(MODULES_ARCH_PATH)/pci/main.o $(MODULES_ARCH_PATH)/pci/bus.o $(MODULES_ARCH_PATH)/pci/device.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
