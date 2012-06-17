MODULES+=modules/arch/x86/acpi/acpi.ko
CLEAN+=modules/arch/x86/acpi/main.o modules/arch/x86/acpi/driver.o

modules/arch/x86/acpi/acpi.ko: lib/memory.o lib/string.o lib/arch/x86/io.o modules/arch/x86/acpi/main.o modules/arch/x86/acpi/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
