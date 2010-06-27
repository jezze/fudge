ASM=nasm
ASMFLAGS=-f bin
GCC=gcc
GCCFLAGS=-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs
LD=ld
LDFLAGS=--oformat binary -Ttext 0x200000 -e main

all:
	@$(GCC) $(GCCFLAGS) date.c -o date.o
	@$(LD) $(LDFLAGS) date.o ../kernel/arch/i386/call.o -o date
	@$(GCC) $(GCCFLAGS) hello.c -o hello.o
	@$(LD) $(LDFLAGS) hello.o ../kernel/arch/i386/call.o -o hello
	@$(GCC) $(GCCFLAGS) ls.c -o ls.o
	@$(LD) $(LDFLAGS) ls.o ../kernel/arch/i386/call.o -o ls
	@$(GCC) $(GCCFLAGS) timer.c -o timer.o
	@$(LD) $(LDFLAGS) timer.o ../kernel/arch/i386/call.o -o timer

clean:
	rm -f *.o
	rm -f hello
	rm -f date
	rm -f ls
	rm -f timer
