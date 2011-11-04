GCC=gcc
GCCFLAGS=-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -m32

.PHONY: all clean kernel

all: kernel

kernel:
	@${GCC} ${GCCFLAGS} error.c -o error.o
	@${GCC} ${GCCFLAGS} event.c -o event.o
	@${GCC} ${GCCFLAGS} initrd.c -o initrd.o
	@${GCC} ${GCCFLAGS} kernel.c -o kernel.o
	@${GCC} ${GCCFLAGS} log.c -o log.o
	@${GCC} ${GCCFLAGS} modules.c -o modules.o
	@${GCC} ${GCCFLAGS} runtime.c -o runtime.o
	@${GCC} ${GCCFLAGS} syscall.c -o syscall.o
	@${GCC} ${GCCFLAGS} vfs.c -o vfs.o

clean:
	@rm -f *.o

