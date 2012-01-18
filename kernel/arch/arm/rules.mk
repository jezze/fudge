arch:
	@${GCC} ${GCCFLAGS} arch/arm/arch.c -o arch/arm/arch.o
	@${ASM} ${ASMFLAGS} arch/arm/cpu.s -o arch/arm/cpu.o
	@${ASM} ${ASMFLAGS} arch/arm/init.s -o arch/arm/init.o

arch-clean:
	@rm -f arch/arm/*.o

