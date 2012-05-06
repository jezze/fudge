include lib/arch/${ARCH}/rules.mk

lib: GCCFLAGS+=-Iinclude

lib: lib-arch
	@${GCC} ${GCCFLAGS} lib/elf.c -o lib/elf.o
	@${GCC} ${GCCFLAGS} lib/flake.c -o lib/flake.o
	@${GCC} ${GCCFLAGS} lib/memory.c -o lib/memory.o
	@${GCC} ${GCCFLAGS} lib/ppm.c -o lib/ppm.o
	@${GCC} ${GCCFLAGS} lib/string.c -o lib/string.o
	@${AR} ${ARFLAGS} lib/libfudge.a lib/*.o lib/arch/${ARCH}/*.o

lib-clean: lib-arch-clean
	@rm -f lib/*.o
	@rm -f lib/libfudge.a

