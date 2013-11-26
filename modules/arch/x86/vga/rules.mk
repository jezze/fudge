MOD:=modules/arch/x86/vga/vga.ko
OBJ:=modules/arch/x86/vga/main.o modules/arch/x86/vga/driver.o modules/arch/x86/vga/timing.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
