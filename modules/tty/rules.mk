MODULES+=modules/tty/tty.ko
CLEAN+=modules/tty/main.o modules/tty/driver.o

modules/tty/tty.ko: lib/memory.o lib/string.o modules/tty/main.o modules/tty/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
