MODULES+=modules/arch/x86/ps2/ps2.ko
MODULES_OBJECTS+=modules/arch/x86/ps2/main.o modules/arch/x86/ps2/bus.o modules/arch/x86/ps2/device.o modules/arch/x86/ps2/kbd_driver.o modules/arch/x86/ps2/mouse_driver.o

modules/arch/x86/ps2/ps2.ko: modules/arch/x86/ps2/main.o modules/arch/x86/ps2/bus.o modules/arch/x86/ps2/device.o modules/arch/x86/ps2/kbd_driver.o modules/arch/x86/ps2/mouse_driver.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
