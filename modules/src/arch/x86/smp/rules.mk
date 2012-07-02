MODULES+=modules/src/arch/x86/smp/smp.ko
CLEAN+=modules/src/arch/x86/smp/main.o modules/src/arch/x86/smp/driver.o

modules/src/arch/x86/smp/smp.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/smp/main.o modules/src/arch/x86/smp/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
