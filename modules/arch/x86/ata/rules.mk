MOD:=modules/arch/x86/ata/ata.ko
OBJ:=modules/arch/x86/ata/main.o modules/arch/x86/ata/driver.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
