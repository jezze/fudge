MODULES+=modules/mbr/mbr.ko
CLEAN+=modules/mbr/main.o modules/mbr/driver.o modules/mbr/device.o

modules/mbr/mbr.ko: lib/memory.o lib/string.o modules/mbr/main.o modules/mbr/driver.o modules/mbr/device.o
	$(LD) $(LDFLAGS) -o $@ $^
