MODULES+=modules/src/arch/x86/rtc/rtc.ko
CLEAN+=modules/src/arch/x86/rtc/main.o modules/src/arch/x86/rtc/device.o

modules/src/arch/x86/rtc/rtc.ko: lib/memory.o lib/string.o lib/arch/x86/io.o modules/src/arch/x86/rtc/main.o modules/src/arch/x86/rtc/device.o
	$(LD) $(LDFLAGS) -o $@ $^
