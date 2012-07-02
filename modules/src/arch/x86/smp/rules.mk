MODULES+=modules/src/arch/x86/smp/smp.ko
CLEAN+=modules/src/arch/x86/smp/main.o modules/src/arch/x86/smp/driver.o

modules/src/arch/x86/smp/smp.ko: lib/memory.o lib/string.o lib/arch/x86/io.o modules/src/arch/x86/smp/main.o modules/src/arch/x86/smp/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
