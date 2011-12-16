all-arm: ARCH=arm
all-arm: GCC=arm-none-eabi-gcc
all-arm: GCCFLAGS=-O2 -I../include/lib -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -L../lib/ -L/usr/lib/gcc/arm-none-eabi/4.6.1/
all-arm: LIB=-lfudge -lgcc
all-x86: ARCH=x86
all-x86: GCC=gcc
all-x86: GCCFLAGS=-O2 -I../include/lib -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -L../lib/ -m32
all-x86: LIB=-lfudge

.PHONY: all clean user

all-arm: all
all-x86: all

all:
	@${GCC} ${GCCFLAGS} cat.c ${LIB} -o cat
	@${GCC} ${GCCFLAGS} cd.c ${LIB} -o cd
	@${GCC} ${GCCFLAGS} clear.c ${LIB} -o clear
	@${GCC} ${GCCFLAGS} date.c ${LIB} -o date
	@${GCC} ${GCCFLAGS} echo.c ${LIB} -o echo
	@${GCC} ${GCCFLAGS} event1.c ${LIB} -o event1
	@${GCC} ${GCCFLAGS} event2.c ${LIB} -o event2
	@${GCC} ${GCCFLAGS} event3.c ${LIB} -o event3
	@${GCC} ${GCCFLAGS} hello.c ${LIB} -o hello
	@${GCC} ${GCCFLAGS} init.c ${LIB} -o init
	@${GCC} ${GCCFLAGS} load.c ${LIB} -o load
	@${GCC} ${GCCFLAGS} ls.c ${LIB} -o ls
	@${GCC} ${GCCFLAGS} reboot.c ${LIB} -o reboot
	@${GCC} ${GCCFLAGS} shell.c ${LIB} -o shell
	@${GCC} ${GCCFLAGS} timer.c ${LIB} -o timer
	@${GCC} ${GCCFLAGS} unload.c ${LIB} -o unload

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

