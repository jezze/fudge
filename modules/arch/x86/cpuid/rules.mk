MODULES+=modules/arch/x86/cpuid/cpuid.ko
MODULES_OBJECTS+=modules/arch/x86/cpuid/main.o modules/arch/x86/cpuid/cpuid.o

modules/arch/x86/cpuid/cpuid.ko: modules/arch/x86/cpuid/main.o modules/arch/x86/cpuid/cpuid.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
