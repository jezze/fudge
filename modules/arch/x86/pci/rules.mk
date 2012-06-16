BUILD+=modules/arch/x86/pci/pci.ko
CLEAN+=$(OBJ_modules/arch/x86/pci/pci.ko) modules/arch/x86/pci/pci.ko

OBJ_modules/arch/x86/pci/pci.ko:=modules/arch/x86/pci/main.o modules/arch/x86/pci/bus.o modules/arch/x86/pci/device.o

modules/arch/x86/pci/pci.ko: lib/memory.o lib/string.o lib/arch/x86/io.o $(OBJ_modules/arch/x86/pci/pci.ko)
	$(LD) $(LDFLAGS) -o $@ $^
