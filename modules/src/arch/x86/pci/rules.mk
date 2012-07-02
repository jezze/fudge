MODULES+=modules/src/arch/x86/pci/pci.ko
CLEAN+=modules/src/arch/x86/pci/main.o modules/src/arch/x86/pci/bus.o modules/src/arch/x86/pci/device.o

modules/src/arch/x86/pci/pci.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/pci/main.o modules/src/arch/x86/pci/bus.o modules/src/arch/x86/pci/device.o
	$(LD) $(LDFLAGS) -o $@ $^
