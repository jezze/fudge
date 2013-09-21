MOD:=modules/network/ipv6/ipv6.ko
OBJ:=modules/network/ipv6/main.o modules/network/ipv6/protocol.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
