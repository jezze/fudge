MOD:=modules/net/arp/arp.ko
OBJ:=modules/net/arp/main.o modules/net/arp/protocol.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
