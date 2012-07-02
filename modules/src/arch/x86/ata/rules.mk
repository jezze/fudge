MODULES+=modules/src/arch/x86/ata/ata.ko
CLEAN+=modules/src/arch/x86/ata/main.o modules/src/arch/x86/ata/bus.o modules/src/arch/x86/ata/device.o

modules/src/arch/x86/ata/ata.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/ata/main.o modules/src/arch/x86/ata/bus.o modules/src/arch/x86/ata/device.o
	$(LD) $(LDFLAGS) -o $@ $^
