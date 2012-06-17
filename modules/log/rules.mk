MODULES+=modules/log/log.ko
CLEAN+=modules/log/main.o modules/log/driver.o

modules/log/log.ko: lib/memory.o lib/string.o modules/log/main.o modules/log/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
