lib-arch:
	@${ASM} ${ASMFLAGS} lib/arch/x86/call.s -o lib/arch/x86/call.o
	@${ASM} ${ASMFLAGS} lib/arch/x86/crt0.s -o lib/arch/x86/crt0.o

lib-arch-clean:
	@rm -f lib/arch/x86/*.o

