BUILD+=modules/arch/x86/rtc/rtc.ko
CLEAN+=$(OBJ_modules/arch/x86/rtc/rtc.ko) modules/arch/x86/rtc/rtc.ko

OBJ_modules/arch/x86/rtc/rtc.ko:=modules/arch/x86/rtc/main.o modules/arch/x86/rtc/device.o

modules/arch/x86/rtc/rtc.ko: lib/memory.o lib/string.o lib/arch/x86/io.o $(OBJ_modules/arch/x86/rtc/rtc.ko)
	$(LD) $(LDFLAGS) -o $@ $^
