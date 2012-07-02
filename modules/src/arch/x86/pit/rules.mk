MODULES+=modules/src/arch/x86/pit/pit.ko
CLEAN+=modules/src/arch/x86/pit/main.o modules/src/arch/x86/pit/driver.o modules/src/arch/x86/pit/device.o

modules/src/arch/x86/pit/pit.ko: lib/memory.o lib/string.o lib/arch/x86/io.o modules/src/arch/x86/pit/main.o modules/src/arch/x86/pit/driver.o modules/src/arch/x86/pit/device.o
	$(LD) $(LDFLAGS) -o $@ $^
