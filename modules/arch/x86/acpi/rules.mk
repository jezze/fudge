MODULES+=modules/arch/x86/acpi/acpi.ko
MODULES_OBJECTS+=modules/arch/x86/acpi/main.o

modules/arch/x86/acpi/acpi.ko: modules/arch/x86/acpi/main.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
