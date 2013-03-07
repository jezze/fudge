MOD:=modules/arch/x86/io/io.ko
OBJ:=modules/arch/x86/io/main.o modules/arch/x86/io/io.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
