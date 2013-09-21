MOD:=modules/network/ipv4/ipv4.ko
OBJ:=modules/network/ipv4/main.o modules/network/ipv4/protocol.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
