MODULES+=modules/ipv4/ipv4.ko
MODULES_OBJECTS+=modules/ipv4/main.o modules/ipv4/protocol.o

modules/ipv4/ipv4.ko: modules/ipv4/main.o modules/ipv4/protocol.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
