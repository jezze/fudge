MOD:=modules/system/system.ko
OBJ:=modules/system/main.o modules/system/backend.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
