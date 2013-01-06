MODULES+=modules/arch/x86/pci/pci.ko
MODULES_OBJECTS+=modules/arch/x86/pci/main.o modules/arch/x86/pci/bus.o modules/arch/x86/pci/device.o

modules/arch/x86/pci/pci.ko: modules/arch/x86/pci/main.o modules/arch/x86/pci/bus.o modules/arch/x86/pci/device.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
