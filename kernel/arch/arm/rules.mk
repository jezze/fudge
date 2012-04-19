kernel-arch:
	@${GCC} ${GCCFLAGS} kernel/arch/arm/arch.c -o kernel/arch/arm/arch.o
	@${ASM} ${ASMFLAGS} kernel/arch/arm/cpu.s -o kernel/arch/arm/cpu.o
	@${ASM} ${ASMFLAGS} kernel/arch/arm/init.s -o kernel/arch/arm/init.o

kernel-arch-clean:
	@rm -f kernel/arch/arm/*.o

