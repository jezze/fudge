MODULES+=modules/ipv6/ipv6.ko
MODULES_OBJECTS+=modules/ipv6/main.o modules/ipv6/protocol.o

modules/ipv6/ipv6.ko: modules/ipv6/main.o modules/ipv6/protocol.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
