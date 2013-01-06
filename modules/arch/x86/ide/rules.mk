MODULES+=modules/arch/x86/ide/ide.ko
MODULES_OBJECTS+=modules/arch/x86/ide/main.o modules/arch/x86/ide/bus.o modules/arch/x86/ide/device.o

modules/arch/x86/ide/ide.ko: modules/arch/x86/ide/main.o modules/arch/x86/ide/bus.o modules/arch/x86/ide/device.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
