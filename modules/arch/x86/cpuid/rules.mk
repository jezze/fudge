MOD:=modules/arch/x86/cpuid/cpuid.ko
OBJ:=modules/arch/x86/cpuid/main.o modules/arch/x86/cpuid/cpuid.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
