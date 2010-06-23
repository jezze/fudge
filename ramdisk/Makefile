ASM=nasm
ASMFLAGS=-f bin
GCC=gcc
GCCFLAGS=-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs
LD=ld
LDFLAGS=--oformat binary -e main

all:
	$(GCC) $(GCCFLAGS) cat.c -o cat.o
	$(LD) $(LDFLAGS) ../lib/io.o ../lib/memory.o ../kernel/screen.o ../kernel/vfs.o cat.o -o cat
	$(GCC) $(GCCFLAGS) ls.c -o ls.o
	$(LD) $(LDFLAGS) ../lib/io.o ../lib/memory.o ../kernel/screen.o ../kernel/vfs.o ls.o -o ls
	$(GCC) $(GCCFLAGS) hello2.c -o hello2.o
	$(LD) $(LDFLAGS) hello2.o ../kernel/arch/i386/call.o -o hello2
	$(ASM) $(ASMFLAGS) hello.s -o hello

clean:
	rm -f *.o
	rm -f cat
	rm -f ls
	rm -f hello
	rm -f hello2
