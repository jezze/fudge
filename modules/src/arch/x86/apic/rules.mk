MODULES+=modules/src/arch/x86/apic/apic.ko
CLEAN+=modules/src/arch/x86/apic/main.o modules/src/arch/x86/apic/driver.o

modules/src/arch/x86/apic/apic.ko: lib/memory.o lib/string.o lib/arch/x86/io.o modules/src/arch/x86/apic/main.o modules/src/arch/x86/apic/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
