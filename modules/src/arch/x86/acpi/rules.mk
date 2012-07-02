MODULES+=modules/src/arch/x86/acpi/acpi.ko
CLEAN+=modules/src/arch/x86/acpi/main.o modules/src/arch/x86/acpi/driver.o

modules/src/arch/x86/acpi/acpi.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/acpi/main.o modules/src/arch/x86/acpi/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
