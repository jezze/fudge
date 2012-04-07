lib-arch:
	@${ASM} ${ASMFLAGS} lib/arch/x86/call.s -o lib/arch/x86/call.o
	@${ASM} ${ASMFLAGS} lib/arch/x86/crt0.s -o lib/arch/x86/crt0.o
	@${ASM} ${ASMFLAGS} lib/arch/x86/io.s -o lib/arch/x86/io.o

lib-arch-clean:
	@rm -f lib/arch/x86/*.o

