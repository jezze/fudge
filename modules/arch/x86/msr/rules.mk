MODULES+=modules/arch/x86/msr/msr.ko
CLEAN+=modules/arch/x86/msr/main.o modules/arch/x86/msr/driver.o modules/arch/x86/msr/msr.o

modules/arch/x86/msr/msr.ko: lib/memory.o lib/string.o lib/arch/x86/io.o modules/arch/x86/msr/main.o modules/arch/x86/msr/driver.o modules/arch/x86/msr/msr.o
	$(LD) $(LDFLAGS) -o $@ $^
