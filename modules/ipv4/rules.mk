MOD:=modules/ipv4/ipv4.ko
OBJ:=modules/ipv4/main.o modules/ipv4/protocol.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
