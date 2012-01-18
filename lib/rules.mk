include lib/arch/${ARCH}/rules.mk

lib: GCCFLAGS+=-Iinclude

lib: lib-arch
	@${GCC} ${GCCFLAGS} lib/file.c -o lib/file.o
	@${GCC} ${GCCFLAGS} lib/memory.c -o lib/memory.o
	@${GCC} ${GCCFLAGS} lib/string.c -o lib/string.o
	@${AR} ${ARFLAGS} lib/libfudge.a lib/arch/${ARCH}/crt0.o lib/arch/${ARCH}/call.o lib/file.o lib/memory.o lib/string.o

lib-clean: lib-arch-clean
	@rm -f lib/*.o
	@rm -f lib/libfudge.a

