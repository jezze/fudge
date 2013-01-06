MODULES+=modules/arch/x86/rtc/rtc.ko
MODULES_OBJECTS+=modules/arch/x86/rtc/main.o modules/arch/x86/rtc/device.o

modules/arch/x86/rtc/rtc.ko: modules/arch/x86/rtc/main.o modules/arch/x86/rtc/device.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
