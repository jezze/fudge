MODULES+=modules/arch/x86/cpuid/cpuid.ko
CLEAN+=modules/arch/x86/cpuid/main.o modules/arch/x86/cpuid/driver.o modules/arch/x86/cpuid/cpuid.o

modules/arch/x86/cpuid/cpuid.ko: lib/memory.o lib/string.o lib/arch/x86/io.o modules/arch/x86/cpuid/main.o modules/arch/x86/cpuid/driver.o modules/arch/x86/cpuid/cpuid.o
	$(LD) $(LDFLAGS) -o $@ $^
