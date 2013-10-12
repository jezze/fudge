MOD:=modules/arch/x86/uart/uart.ko
OBJ:=modules/arch/x86/uart/main.o modules/arch/x86/uart/driver.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
