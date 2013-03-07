MOD:=modules/arch/x86/multi/multi.ko
OBJ:=modules/arch/x86/multi/main.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
