MODULES+=modules/arch/x86/ps2/ps2.ko
CLEAN+=modules/arch/x86/ps2/main.o modules/arch/x86/ps2/kbd_driver.o modules/arch/x86/ps2/kbd_device.o modules/arch/x86/ps2/mouse_driver.o modules/arch/x86/ps2/mouse_device.o

modules/arch/x86/ps2/ps2.ko: lib/memory.o lib/string.o lib/arch/x86/io.o modules/arch/x86/ps2/main.o modules/arch/x86/ps2/kbd_driver.o modules/arch/x86/ps2/kbd_device.o modules/arch/x86/ps2/mouse_driver.o modules/arch/x86/ps2/mouse_device.o
	$(LD) $(LDFLAGS) -o $@ $^
