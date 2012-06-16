BUILD+=modules/nodefs/nodefs.ko
CLEAN+=$(OBJ_modules/nodefs/nodefs.ko) modules/nodefs/nodefs.ko

OBJ_modules/nodefs/nodefs.ko:=modules/nodefs/main.o modules/nodefs/driver.o

modules/nodefs/nodefs.ko: LDFLAGS+=-Tmodules/linker.ld -r lib/memory.o lib/string.o
modules/nodefs/nodefs.ko: lib $(OBJ_modules/nodefs/nodefs.ko)
	$(LD) $(LDFLAGS) -o $@ $(OBJ_modules/nodefs/nodefs.ko)

