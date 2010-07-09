ASM=nasm
ASMFLAGS=-f elf
GCC=gcc
GCCFLAGS=-c -O2 -I../include/lib -Wall -ffreestanding -nostdlib -nostartfiles -nodefaultlibs
LD=ld
LDFLAGS=--oformat binary -Ttext 0x200000 -e main

all:
	@$(GCC) $(GCCFLAGS) cat.c -o cat.o
	@$(LD) $(LDFLAGS) cat.o ../lib/call.o -o cat
	@$(ASM) $(ASMFLAGS) cpu.s -o cpus.o
	@$(GCC) $(GCCFLAGS) cpu.c -o cpu.o
	@$(LD) $(LDFLAGS) cpu.o cpus.o ../lib/call.o -o cpu
	@$(GCC) $(GCCFLAGS) date.c -o date.o
	@$(LD) $(LDFLAGS) date.o ../lib/call.o -o date
	@$(GCC) $(GCCFLAGS) hello.c -o hello.o
	@$(LD) $(LDFLAGS) hello.o ../lib/call.o -o hello
	@$(GCC) $(GCCFLAGS) ls.c -o ls.o
	@$(LD) $(LDFLAGS) ls.o ../lib/call.o -o ls
	@$(GCC) $(GCCFLAGS) timer.c -o timer.o
	@$(LD) $(LDFLAGS) timer.o ../lib/call.o -o timer

clean:
	@rm -f *.o
	@rm -f cat
	@rm -f cpu
	@rm -f date
	@rm -f hello
	@rm -f ls
	@rm -f timer

