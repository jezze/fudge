MODULES+=modules/src/log/log.ko
CLEAN+=modules/src/log/main.o modules/src/log/driver.o

modules/src/log/log.ko: lib/src/memory.o lib/src/string.o modules/src/log/main.o modules/src/log/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
