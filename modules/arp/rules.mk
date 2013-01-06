MODULES+=modules/arp/arp.ko
MODULES_OBJECTS+=modules/arp/main.o modules/arp/protocol.o

modules/arp/arp.ko: modules/arp/main.o modules/arp/protocol.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
