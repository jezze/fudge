MODULES+=modules/nodefs/nodefs.ko
CLEAN+=modules/nodefs/main.o modules/nodefs/driver.o

modules/nodefs/nodefs.ko: lib/memory.o lib/string.o modules/nodefs/main.o modules/nodefs/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
