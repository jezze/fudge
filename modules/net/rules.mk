MODULES+=modules/net/net.ko
MODULES_OBJECTS+=modules/net/main.o

modules/net/net.ko: modules/net/main.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
