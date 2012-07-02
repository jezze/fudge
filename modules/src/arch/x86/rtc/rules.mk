MODULES+=modules/src/arch/x86/rtc/rtc.ko
CLEAN+=modules/src/arch/x86/rtc/main.o modules/src/arch/x86/rtc/device.o

modules/src/arch/x86/rtc/rtc.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/rtc/main.o modules/src/arch/x86/rtc/device.o
	$(LD) $(LDFLAGS) -o $@ $^
