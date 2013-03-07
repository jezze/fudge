MOD:=modules/arch/x86/atapi/atapi.ko
OBJ:=modules/arch/x86/atapi/main.o modules/arch/x86/atapi/driver.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
