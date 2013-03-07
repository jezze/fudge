MOD:=modules/arch/x86/fpu/fpu.ko
OBJ:=modules/arch/x86/fpu/main.o modules/arch/x86/fpu/fpu.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
