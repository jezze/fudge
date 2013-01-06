MODULES+=modules/system/system.ko
MODULES_OBJECTS+=modules/system/main.o modules/system/filesystem.o

modules/system/system.ko: modules/system/main.o modules/system/filesystem.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
