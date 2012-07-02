MODULES+=modules/src/arch/x86/cpuid/cpuid.ko
CLEAN+=modules/src/arch/x86/cpuid/main.o modules/src/arch/x86/cpuid/driver.o modules/src/arch/x86/cpuid/cpuid.o

modules/src/arch/x86/cpuid/cpuid.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/cpuid/main.o modules/src/arch/x86/cpuid/driver.o modules/src/arch/x86/cpuid/cpuid.o
	$(LD) $(LDFLAGS) -o $@ $^
