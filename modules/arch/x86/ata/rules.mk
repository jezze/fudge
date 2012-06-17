MODULES+=modules/arch/x86/ata/ata.ko
CLEAN+=$(OBJ_modules/arch/x86/ata/ata.ko) modules/arch/x86/ata/ata.ko

OBJ_modules/arch/x86/ata/ata.ko:=modules/arch/x86/ata/main.o modules/arch/x86/ata/bus.o modules/arch/x86/ata/device.o

modules/arch/x86/ata/ata.ko: lib/memory.o lib/string.o lib/arch/x86/io.o $(OBJ_modules/arch/x86/ata/ata.ko)
	$(LD) $(LDFLAGS) -o $@ $^
