MODULES+=modules/log/log.ko
CLEAN+=$(OBJ_modules/log/log.ko) modules/log/log.ko

OBJ_modules/log/log.ko:=modules/log/main.o modules/log/driver.o

modules/log/log.ko: lib/memory.o lib/string.o $(OBJ_modules/log/log.ko)
	$(LD) $(LDFLAGS) -o $@ $^
