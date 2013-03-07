MOD:=modules/arch/x86/pic/pic.ko
OBJ:=modules/arch/x86/pic/main.o modules/arch/x86/pic/pic.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
