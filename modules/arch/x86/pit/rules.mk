MODULES+=modules/arch/x86/pit/pit.ko
MODULES_OBJECTS+=modules/arch/x86/pit/main.o modules/arch/x86/pit/driver.o modules/arch/x86/pit/device.o

modules/arch/x86/pit/pit.ko: modules/arch/x86/pit/main.o modules/arch/x86/pit/driver.o modules/arch/x86/pit/device.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
