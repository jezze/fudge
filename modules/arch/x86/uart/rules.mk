MOD:=modules/arch/x86/uart/uart.ko
OBJ:=modules/arch/x86/uart/main.o modules/arch/x86/uart/driver.o modules/arch/x86/uart/device.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
