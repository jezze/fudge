MODULES+=modules/block/block.ko
MODULES_OBJECTS+=modules/block/main.o

modules/block/block.ko: modules/block/main.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
