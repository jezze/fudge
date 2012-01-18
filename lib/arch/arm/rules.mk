lib-arch:
	@${GCC} ${GCCFLAGS} lib/arch/arm/call.c -o lib/arch/arm/call.o
	@${ASM} ${ASMFLAGS} lib/arch/arm/crt0.s -o lib/arch/arm/crt0.o

lib-arch-clean:
	@rm -f lib/arch/arm/*.o

