MOD:=modules/arch/x86/ide/ide.ko
OBJ:=modules/arch/x86/ide/main.o modules/arch/x86/ide/bus.o modules/arch/x86/ide/device.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
