MOD:=modules/arch/x86/bga/bga.ko
OBJ:=modules/arch/x86/bga/main.o modules/arch/x86/bga/driver.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
