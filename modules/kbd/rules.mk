MODULES+=modules/kbd/kbd.ko
MODULES_OBJECTS+=modules/kbd/main.o

modules/kbd/kbd.ko: modules/kbd/main.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
