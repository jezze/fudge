MOD:=modules/arch/x86/smp/smp.ko
OBJ:=modules/arch/x86/smp/main.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
