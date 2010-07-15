ASM=nasm
ASMFLAGS=-f elf
GCC=gcc
GCCFLAGS=-c -O2 -I../include/lib -Wall -ffreestanding -nostdlib -nostartfiles -nodefaultlibs
LD=ld
LDFLAGS=--oformat binary -Ttext 0x200000 -e main

all:
	@$(GCC) $(GCCFLAGS) cat.c -o cat.o
	@$(LD) $(LDFLAGS) cat.o ../lib/string.o ../lib/vfs.o ../lib/call.o ../lib/calls.o -o cat
	@$(ASM) $(ASMFLAGS) cpu.s -o cpus.o
	@$(GCC) $(GCCFLAGS) cpu.c -o cpu.o
	@$(LD) $(LDFLAGS) cpu.o cpus.o ../lib/string.o ../lib/vfs.o ../lib/call.o ../lib/calls.o -o cpu
	@$(GCC) $(GCCFLAGS) date.c -o date.o
	@$(LD) $(LDFLAGS) date.o ../lib/string.o ../lib/vfs.o ../lib/call.o ../lib/calls.o -o date
	@$(GCC) $(GCCFLAGS) echo.c -o echo.o
	@$(LD) $(LDFLAGS) echo.o ../lib/string.o ../lib/vfs.o ../lib/call.o ../lib/calls.o -o echo
	@$(GCC) $(GCCFLAGS) hello.c -o hello.o
	@$(LD) $(LDFLAGS) hello.o ../lib/string.o ../lib/vfs.o ../lib/call.o ../lib/calls.o -o hello
	@$(GCC) $(GCCFLAGS) ls.c -o ls.o
	@$(LD) $(LDFLAGS) ls.o ../lib/string.o ../lib/vfs.o ../lib/call.o ../lib/calls.o -o ls
	@$(GCC) $(GCCFLAGS) reboot.c -o reboot.o
	@$(LD) $(LDFLAGS) reboot.o ../lib/string.o ../lib/vfs.o ../lib/call.o ../lib/calls.o -o reboot
	@$(GCC) $(GCCFLAGS) timer.c -o timer.o
	@$(LD) $(LDFLAGS) timer.o ../lib/string.o ../lib/vfs.o ../lib/call.o ../lib/calls.o -o timer

clean:
	@rm -f *.o
	@rm -f cat
	@rm -f cpu
	@rm -f date
	@rm -f echo
	@rm -f hello
	@rm -f ls
	@rm -f reboot
	@rm -f timer

