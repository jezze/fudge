MODULES+=modules/src/mbr/mbr.ko
CLEAN+=modules/src/mbr/main.o modules/src/mbr/driver.o modules/src/mbr/device.o

modules/src/mbr/mbr.ko: lib/src/memory.o lib/src/string.o modules/src/mbr/main.o modules/src/mbr/driver.o modules/src/mbr/device.o
	$(LD) $(LDFLAGS) -o $@ $^
