MODULES+=modules/arch/x86/apic/apic.ko
CLEAN+=modules/arch/x86/apic/main.o modules/arch/x86/apic/driver.o

modules/arch/x86/apic/apic.ko: lib/memory.o lib/string.o lib/arch/x86/io.o modules/arch/x86/apic/main.o modules/arch/x86/apic/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
