ASM=nasm
ASMFLAGS=-f elf
GCC=gcc
GCCFLAGS=-c -O2 -I../include/lib -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -m32
LD=ld
LDFLAGS=-e main -melf_i386 ../lib/file.o ../lib/memory.o ../lib/string.o ../kernel/arch/x86/calls.o

.PHONY: all clean user

all: user

user:
	@${ASM} ${ASMFLAGS} cpu.s -o cpus.o
	@${GCC} ${GCCFLAGS} cat.c -o cat.o
	@${GCC} ${GCCFLAGS} cd.c -o cd.o
	@${GCC} ${GCCFLAGS} clear.c -o clear.o
	@${GCC} ${GCCFLAGS} cpu.c -o cpu.o
	@${GCC} ${GCCFLAGS} date.c -o date.o
	@${GCC} ${GCCFLAGS} echo.c -o echo.o
	@${GCC} ${GCCFLAGS} event1.c -o event1.o
	@${GCC} ${GCCFLAGS} event2.c -o event2.o
	@${GCC} ${GCCFLAGS} event3.c -o event3.o
	@${GCC} ${GCCFLAGS} hello.c -o hello.o
	@${GCC} ${GCCFLAGS} init.c -o init.o
	@${GCC} ${GCCFLAGS} load.c -o load.o
	@${GCC} ${GCCFLAGS} ls.c -o ls.o
	@${GCC} ${GCCFLAGS} reboot.c -o reboot.o
	@${GCC} ${GCCFLAGS} shell.c -o shell.o
	@${GCC} ${GCCFLAGS} timer.c -o timer.o
	@${GCC} ${GCCFLAGS} unload.c -o unload.o
	@${GCC} ${GCCFLAGS} vga.c -o vga.o
	@${LD} ${LDFLAGS} cat.o -o cat
	@${LD} ${LDFLAGS} cd.o -o cd
	@${LD} ${LDFLAGS} clear.o -o clear
	@${LD} ${LDFLAGS} cpu.o cpus.o -o cpu
	@${LD} ${LDFLAGS} date.o -o date
	@${LD} ${LDFLAGS} echo.o -o echo
	@${LD} ${LDFLAGS} event1.o -o event1
	@${LD} ${LDFLAGS} event2.o -o event2
	@${LD} ${LDFLAGS} event3.o -o event3
	@${LD} ${LDFLAGS} hello.o -o hello
	@${LD} ${LDFLAGS} init.o -o init
	@${LD} ${LDFLAGS} load.o -o load
	@${LD} ${LDFLAGS} ls.o -o ls
	@${LD} ${LDFLAGS} reboot.o -o reboot
	@${LD} ${LDFLAGS} shell.o -o shell
	@${LD} ${LDFLAGS} timer.o -o timer
	@${LD} ${LDFLAGS} unload.o -o unload
	@${LD} ${LDFLAGS} vga.o -o vga

clean:
	@rm -f *.o
	@rm -f cat
	@rm -f cd
	@rm -f clear
	@rm -f cpu
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
	@rm -f vga

