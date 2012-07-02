MODULES+=modules/src/arch/x86/msr/msr.ko
CLEAN+=modules/src/arch/x86/msr/main.o modules/src/arch/x86/msr/driver.o modules/src/arch/x86/msr/msr.o

modules/src/arch/x86/msr/msr.ko: lib/memory.o lib/string.o lib/arch/x86/io.o modules/src/arch/x86/msr/main.o modules/src/arch/x86/msr/driver.o modules/src/arch/x86/msr/msr.o
	$(LD) $(LDFLAGS) -o $@ $^
