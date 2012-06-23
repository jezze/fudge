MODULES+=modules/arch/x86/fpu/fpu.ko
CLEAN+=modules/arch/x86/fpu/main.o modules/arch/x86/fpu/driver.o modules/arch/x86/fpu/fpu.o

modules/arch/x86/fpu/fpu.ko: lib/memory.o lib/string.o lib/arch/x86/io.o modules/arch/x86/fpu/main.o modules/arch/x86/fpu/driver.o modules/arch/x86/fpu/fpu.o
	$(LD) $(LDFLAGS) -o $@ $^
