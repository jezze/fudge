MOD:=modules/arch/x86/pit/pit.ko
OBJ:=modules/arch/x86/pit/main.o modules/arch/x86/pit/driver.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
