MODULES+=modules/src/arch/x86/fpu/fpu.ko
CLEAN+=modules/src/arch/x86/fpu/main.o modules/src/arch/x86/fpu/driver.o modules/src/arch/x86/fpu/fpu.o

modules/src/arch/x86/fpu/fpu.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/fpu/main.o modules/src/arch/x86/fpu/driver.o modules/src/arch/x86/fpu/fpu.o
	$(LD) $(LDFLAGS) -o $@ $^
