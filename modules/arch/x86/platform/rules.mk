MOD:=modules/arch/x86/platform/platform.ko
OBJ:=modules/arch/x86/platform/main.o modules/arch/x86/platform/bus.o modules/arch/x86/platform/device.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
