MOD:=modules/ipv6/ipv6.ko
OBJ:=modules/ipv6/main.o modules/ipv6/protocol.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
