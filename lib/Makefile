all-arm: ARCH=arm
all-arm: GCC=arm-none-eabi-gcc
all-arm: GCCFLAGS=-c -O2 -I../include -Wall -Wextra -nostdlib -nostartfiles -ffreestanding -std=gnu99
all-x86: ARCH=x86
all-x86: GCC=gcc
all-x86: GCCFLAGS=-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -m32

AR=ar
ARFLAGS=rvs

DIR_SOURCE_ARCH=arch/${ARCH}

.PHONY: all clean lib

all: all-x86
all-arm: lib
all-x86: lib

lib:
	@make -C ${DIR_SOURCE_ARCH}/
	@${GCC} ${GCCFLAGS} file.c -o file.o
	@${GCC} ${GCCFLAGS} memory.c -o memory.o
	@${GCC} ${GCCFLAGS} string.c -o string.o
	@${AR} ${ARFLAGS} libfudge.a ${DIR_SOURCE_ARCH}/crt0.o ${DIR_SOURCE_ARCH}/call.o file.o memory.o string.o

clean:
	@make -C arch/arm/ clean
	@make -C arch/x86/ clean
	@rm -f *.o
	@rm -f libfudge.a

