MOD:=modules/net/ipv6/ipv6.ko
OBJ:=modules/net/ipv6/main.o modules/net/ipv6/protocol.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
