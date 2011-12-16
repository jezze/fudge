all-arm: ARCH=arm
all-arm: GCC=arm-none-eabi-gcc
all-arm: GCCFLAGS=-O2 -I../include/lib -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -L../lib/ -L/usr/lib/gcc/arm-none-eabi/4.6.1/ -lfudge -lgcc
all-x86: ARCH=x86
all-x86: GCC=gcc
all-x86: GCCFLAGS=-O2 -I../include/lib -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -L../lib/ -m32

.PHONY: all clean user

all-arm: all
all-x86: all

all:
	@${GCC} ${GCCFLAGS} cat.c -lfudge -o cat
	@${GCC} ${GCCFLAGS} cd.c -lfudge -o cd
	@${GCC} ${GCCFLAGS} clear.c -lfudge -o clear
	@${GCC} ${GCCFLAGS} date.c -lfudge -o date
	@${GCC} ${GCCFLAGS} echo.c -lfudge -o echo
	@${GCC} ${GCCFLAGS} event1.c -lfudge -o event1
	@${GCC} ${GCCFLAGS} event2.c -lfudge -o event2
	@${GCC} ${GCCFLAGS} event3.c -lfudge -o event3
	@${GCC} ${GCCFLAGS} hello.c -lfudge -o hello
	@${GCC} ${GCCFLAGS} init.c -lfudge -o init
	@${GCC} ${GCCFLAGS} load.c -lfudge -o load
	@${GCC} ${GCCFLAGS} ls.c -lfudge -o ls
	@${GCC} ${GCCFLAGS} reboot.c -lfudge -o reboot
	@${GCC} ${GCCFLAGS} shell.c -lfudge -o shell
	@${GCC} ${GCCFLAGS} timer.c -lfudge -o timer
	@${GCC} ${GCCFLAGS} unload.c -lfudge -o unload

clean:
	@rm -f cat
	@rm -f cd
	@rm -f clear
	@rm -f date
	@rm -f echo
	@rm -f event1
	@rm -f event2
	@rm -f event3
	@rm -f hello
	@rm -f init
	@rm -f load
	@rm -f ls
	@rm -f reboot
	@rm -f shell
	@rm -f timer
	@rm -f unload

