MOD:=modules/arch/x86/ps2/ps2.ko
OBJ:=modules/arch/x86/ps2/main.o modules/arch/x86/ps2/bus.o modules/arch/x86/ps2/device.o modules/arch/x86/ps2/kbd_driver.o modules/arch/x86/ps2/mouse_driver.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
