BUILD+=modules/mbr/mbr.ko
CLEAN+=$(OBJ_modules/mbr/mbr.ko) modules/mbr/mbr.ko

OBJ_modules/mbr/mbr.ko:=modules/mbr/main.o modules/mbr/driver.o modules/mbr/device.o

modules/mbr/mbr.ko: lib/memory.o lib/string.o $(OBJ_modules/mbr/mbr.ko)
	$(LD) $(LDFLAGS) -o $@ $^
