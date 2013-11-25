MOD:=modules/arch/x86/cirrus/cirrus.ko
OBJ:=modules/arch/x86/cirrus/main.o modules/arch/x86/cirrus/driver.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
