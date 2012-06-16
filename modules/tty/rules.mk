BUILD+=modules/tty/tty.ko
CLEAN+=$(OBJ_modules/tty/tty.ko) modules/tty/tty.ko

OBJ_modules/tty/tty.ko:=modules/tty/main.o modules/tty/driver.o

modules/tty/tty.ko: lib/memory.o lib/string.o $(OBJ_modules/tty/tty.ko)
	$(LD) $(LDFLAGS) -o $@ $^
