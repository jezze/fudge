MODULES+=modules/arch/x86/uart/uart.ko
MODULES_OBJECTS+=modules/arch/x86/uart/main.o modules/arch/x86/uart/driver.o modules/arch/x86/uart/device.o

modules/arch/x86/uart/uart.ko: modules/arch/x86/uart/main.o modules/arch/x86/uart/driver.o modules/arch/x86/uart/device.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
