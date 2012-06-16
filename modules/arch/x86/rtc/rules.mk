BUILD+=modules/arch/x86/rtc/rtc.ko
CLEAN+=$(OBJ_modules/arch/x86/rtc/rtc.ko) modules/arch/x86/rtc/rtc.ko

OBJ_modules/arch/x86/rtc/rtc.ko:=modules/arch/x86/rtc/main.o modules/arch/x86/rtc/device.o

modules/arch/x86/rtc/rtc.ko: LDFLAGS+=-Tmodules/linker.ld -r lib/memory.o lib/string.o lib/arch/x86/io.o
modules/arch/x86/rtc/rtc.ko: lib $(OBJ_modules/arch/x86/rtc/rtc.ko)
	$(LD) $(LDFLAGS) -o $@ $(OBJ_modules/arch/x86/rtc/rtc.ko)

