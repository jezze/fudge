MOD:=modules/arch/x86/msr/msr.ko
OBJ:=modules/arch/x86/msr/main.o modules/arch/x86/msr/msr.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
