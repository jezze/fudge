MODULES+=modules/arch/x86/apic/apic.ko
MODULES_OBJECTS+=modules/arch/x86/apic/main.o

modules/arch/x86/apic/apic.ko: modules/arch/x86/apic/main.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
