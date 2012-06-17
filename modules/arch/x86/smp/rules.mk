MODULES+=modules/arch/x86/smp/smp.ko
CLEAN+=modules/arch/x86/smp/main.o modules/arch/x86/smp/driver.o

modules/arch/x86/smp/smp.ko: lib/memory.o lib/string.o lib/arch/x86/io.o modules/arch/x86/smp/main.o modules/arch/x86/smp/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
