MOD:=modules/arch/x86/rtl8139/rtl8139.ko
OBJ:=modules/arch/x86/rtl8139/main.o modules/arch/x86/rtl8139/driver.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
