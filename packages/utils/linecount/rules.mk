BIN:=packages/utils/linecount/linecount
OBJ:=packages/utils/linecount/linecount.o

$(BIN): $(OBJ) $(LIBFUDGE) $(EXTRA)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN)
PACKAGES_OBJECTS+=$(OBJ)
