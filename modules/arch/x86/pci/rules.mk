BUILD+=modules/arch/x86/pci/pci.ko
CLEAN+=$(OBJ_modules/arch/x86/pci/pci.ko) modules/arch/x86/pci/pci.ko

OBJ_modules/arch/x86/pci/pci.ko:=modules/arch/x86/pci/main.o modules/arch/x86/pci/bus.o modules/arch/x86/pci/device.o

modules/arch/x86/pci/pci.ko: GCCFLAGS+=-Iinclude
modules/arch/x86/pci/pci.ko: LDFLAGS+=-Tmodules/linker.ld -r lib/memory.o lib/string.o lib/arch/x86/io.o
modules/arch/x86/pci/pci.ko: lib $(OBJ_modules/arch/x86/pci/pci.ko)
	$(LD) $(LDFLAGS) -o $@ $(OBJ_modules/arch/x86/pci/pci.ko)

