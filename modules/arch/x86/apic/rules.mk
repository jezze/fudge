MOD:=modules/arch/x86/apic/apic.ko
OBJ:=modules/arch/x86/apic/main.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
