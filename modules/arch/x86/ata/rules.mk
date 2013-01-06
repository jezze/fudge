MODULES+=modules/arch/x86/ata/ata.ko
MODULES_OBJECTS+=modules/arch/x86/ata/main.o modules/arch/x86/ata/bus.o modules/arch/x86/ata/device.o modules/arch/x86/ata/driver.o

modules/arch/x86/ata/ata.ko: modules/arch/x86/ata/main.o modules/arch/x86/ata/bus.o modules/arch/x86/ata/device.o modules/arch/x86/ata/driver.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
