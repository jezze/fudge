MOD:=modules/net/ipv4/ipv4.ko
OBJ:=modules/net/ipv4/main.o modules/net/ipv4/protocol.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
