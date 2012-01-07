include ../rules.mk
include ../${ARCH}.mk

GCCFLAGS+=-I../include

.PHONY: all clean lib

all: all-x86
all-arm: lib
all-x86: lib

lib:
	@make -C arch/${ARCH}/
	@${GCC} ${GCCFLAGS} file.c -o file.o
	@${GCC} ${GCCFLAGS} memory.c -o memory.o
	@${GCC} ${GCCFLAGS} string.c -o string.o
	@${AR} ${ARFLAGS} libfudge.a arch/${ARCH}/crt0.o arch/${ARCH}/call.o file.o memory.o string.o

clean:
	@make -C arch/arm/ clean
	@make -C arch/x86/ clean
	@rm -f *.o
	@rm -f libfudge.a

