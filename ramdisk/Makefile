ASM=nasm
ASMFLAGS=-f bin
GCC=gcc
GCCFLAGS=-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs
LD=ld
LDFLAGS=--oformat binary

all:
	$(GCC) $(GCCFLAGS) hello2.c -o hello2.o
	$(LD) $(LDFLAGS) hello2.o -o hello2
	$(ASM) $(ASMFLAGS) hello.s -o hello

clean:
	rm -f *.o
	rm -f hello
	rm -f hello2
