MOD:=modules/temp/temp.ko
OBJ:=modules/temp/main.o modules/temp/filesystem.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
