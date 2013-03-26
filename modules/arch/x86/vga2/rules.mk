MOD:=modules/arch/x86/vga2/vga2.ko
OBJ:=modules/arch/x86/vga2/main.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
