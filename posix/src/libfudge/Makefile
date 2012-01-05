all: GCC=gcc
all: GCCFLAGS=-c -O2 -I../../include -I../../../include/lib -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -m32
all: LD=ld
all: LDFLAGS=-melf_i386
all: AR=ar
all: ARFLAGS=rvs

.PHONY: all clean

all:
	@${GCC} ${GCCFLAGS} _exit.c -o _exit.o
	@${GCC} ${GCCFLAGS} chown.c -o chown.o
	@${GCC} ${GCCFLAGS} close.c -o close.o
	@${GCC} ${GCCFLAGS} environ.c -o environ.o
	@${GCC} ${GCCFLAGS} errno.c -o errno.o
	@${GCC} ${GCCFLAGS} execve.c -o execve.o
	@${GCC} ${GCCFLAGS} fork.c -o fork.o
	@${GCC} ${GCCFLAGS} fstat.c -o fstat.o
	@${GCC} ${GCCFLAGS} getpid.c -o getpid.o
	#@${GCC} ${GCCFLAGS} gettod.c -o gettod.o
	@${GCC} ${GCCFLAGS} isatty.c -o isatty.o
	@${GCC} ${GCCFLAGS} kill.c -o kill.o
	@${GCC} ${GCCFLAGS} link.c -o link.o
	@${GCC} ${GCCFLAGS} lseek.c -o lseek.o
	@${GCC} ${GCCFLAGS} open.c -o open.o
	@${GCC} ${GCCFLAGS} read.c -o read.o
	@${GCC} ${GCCFLAGS} readlink.c -o readlink.o
	@${GCC} ${GCCFLAGS} sbrk.c -o sbrk.o
	@${GCC} ${GCCFLAGS} stat.c -o stat.o
	@${GCC} ${GCCFLAGS} symlink.c -o symlink.o
	@${GCC} ${GCCFLAGS} times.c -o times.o
	@${GCC} ${GCCFLAGS} unlink.c -o unlink.o
	@${GCC} ${GCCFLAGS} wait.c -o wait.o
	@${GCC} ${GCCFLAGS} write.c -o write.o
	@${AR} ${ARFLAGS} ../../lib/libfudge.a \
        _exit.o chown.o close.o environ.o errno.o execve.o fork.o fstat.o getpid.o isatty.o kill.o \
        link.o lseek.o open.o read.o readlink.o sbrk.o stat.o symlink.o times.o unlink.o wait.o write.o \
        ../../../lib/arch/x86/crt0.o ../../../lib/arch/x86/call.o

clean:
	@rm -f *.o
	@rm -f ../../lib/libfudge.a

