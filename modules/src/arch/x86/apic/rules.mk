MODULES+=modules/src/arch/x86/apic/apic.ko
CLEAN+=modules/src/arch/x86/apic/main.o modules/src/arch/x86/apic/driver.o

modules/src/arch/x86/apic/apic.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/apic/main.o modules/src/arch/x86/apic/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
