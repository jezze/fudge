BUILD+=modules/arch/x86/cpuid/cpuid.ko
CLEAN+=$(OBJ_modules/arch/x86/cpuid/cpuid.ko) modules/arch/x86/cpuid/cpuid.ko

OBJ_modules/arch/x86/cpuid/cpuid.ko:=modules/arch/x86/cpuid/main.o modules/arch/x86/cpuid/driver.o modules/arch/x86/cpuid/cpuid.o

modules/arch/x86/cpuid/cpuid.ko: lib/memory.o lib/string.o lib/arch/x86/io.o $(OBJ_modules/arch/x86/cpuid/cpuid.ko)
	$(LD) $(LDFLAGS) -o $@ $^
