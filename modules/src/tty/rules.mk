MODULES+=modules/src/tty/tty.ko
CLEAN+=modules/src/tty/main.o modules/src/tty/driver.o

modules/src/tty/tty.ko: lib/memory.o lib/string.o modules/src/tty/main.o modules/src/tty/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
