MODULES+=modules/base/base.ko
MODULES_OBJECTS+=modules/base/main.o

modules/base/base.ko: modules/base/main.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
