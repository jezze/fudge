MODULES+=modules/mouse/mouse.ko
MODULES_OBJECTS+=modules/mouse/main.o

modules/mouse/mouse.ko: modules/mouse/main.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
