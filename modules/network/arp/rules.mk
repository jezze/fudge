MOD:=modules/network/arp/arp.ko
OBJ:=modules/network/arp/main.o modules/network/arp/protocol.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
