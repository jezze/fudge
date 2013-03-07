MOD:=modules/arch/x86/acpi/acpi.ko
OBJ:=modules/arch/x86/acpi/main.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
