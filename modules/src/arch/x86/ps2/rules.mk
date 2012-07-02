MODULES+=modules/src/arch/x86/ps2/ps2.ko
CLEAN+=modules/src/arch/x86/ps2/main.o modules/src/arch/x86/ps2/kbd_driver.o modules/src/arch/x86/ps2/kbd_device.o modules/src/arch/x86/ps2/mouse_driver.o modules/src/arch/x86/ps2/mouse_device.o

modules/src/arch/x86/ps2/ps2.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/ps2/main.o modules/src/arch/x86/ps2/kbd_driver.o modules/src/arch/x86/ps2/kbd_device.o modules/src/arch/x86/ps2/mouse_driver.o modules/src/arch/x86/ps2/mouse_device.o
	$(LD) $(LDFLAGS) -o $@ $^
