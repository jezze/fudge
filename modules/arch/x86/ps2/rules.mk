BUILD+=modules/arch/x86/ps2/ps2.ko
CLEAN+=$(OBJ_modules/arch/x86/ps2/ps2.ko) modules/arch/x86/ps2/ps2.ko

OBJ_modules/arch/x86/ps2/ps2.ko:=modules/arch/x86/ps2/main.o modules/arch/x86/ps2/kbd_driver.o modules/arch/x86/ps2/kbd_device.o modules/arch/x86/ps2/mouse_driver.o modules/arch/x86/ps2/mouse_device.o

modules/arch/x86/ps2/ps2.ko: LDFLAGS+=-Tmodules/linker.ld -r lib/memory.o lib/string.o lib/arch/x86/io.o
modules/arch/x86/ps2/ps2.ko: lib $(OBJ_modules/arch/x86/ps2/ps2.ko)
	$(LD) $(LDFLAGS) -o $@ $(OBJ_modules/arch/x86/ps2/ps2.ko)

