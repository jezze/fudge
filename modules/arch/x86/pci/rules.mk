MOD:=modules/arch/x86/pci/pci.ko
OBJ:=modules/arch/x86/pci/main.o modules/arch/x86/pci/bus.o modules/arch/x86/pci/device.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
