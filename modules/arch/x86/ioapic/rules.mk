MODULES+=modules/arch/x86/ioapic/ioapic.ko
CLEAN+=modules/arch/x86/ioapic/main.o modules/arch/x86/ioapic/driver.o

modules/arch/x86/ioapic/ioapic.ko: lib/memory.o lib/string.o lib/arch/x86/io.o modules/arch/x86/ioapic/main.o modules/arch/x86/ioapic/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
