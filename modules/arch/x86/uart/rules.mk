MODULES+=modules/arch/x86/uart/uart.ko
CLEAN+=modules/arch/x86/uart/main.o modules/arch/x86/uart/driver.o modules/arch/x86/uart/device.o

modules/arch/x86/uart/uart.ko: lib/memory.o lib/string.o lib/arch/x86/io.o modules/arch/x86/uart/main.o modules/arch/x86/uart/driver.o modules/arch/x86/uart/device.o
	$(LD) $(LDFLAGS) -o $@ $^
