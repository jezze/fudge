MODULES+=modules/arch/x86/pci/pci.ko
CLEAN+=modules/arch/x86/pci/main.o modules/arch/x86/pci/bus.o modules/arch/x86/pci/device.o

modules/arch/x86/pci/pci.ko: lib/memory.o lib/string.o lib/arch/x86/io.o modules/arch/x86/pci/main.o modules/arch/x86/pci/bus.o modules/arch/x86/pci/device.o
	$(LD) $(LDFLAGS) -o $@ $^
