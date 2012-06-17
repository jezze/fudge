MODULES+=modules/arch/x86/rtc/rtc.ko
CLEAN+=modules/arch/x86/rtc/main.o modules/arch/x86/rtc/device.o

modules/arch/x86/rtc/rtc.ko: lib/memory.o lib/string.o lib/arch/x86/io.o modules/arch/x86/rtc/main.o modules/arch/x86/rtc/device.o
	$(LD) $(LDFLAGS) -o $@ $^
