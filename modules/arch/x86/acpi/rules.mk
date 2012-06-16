BUILD+=modules/arch/x86/acpi/acpi.ko
CLEAN+=$(OBJ_modules/arch/x86/acpi/acpi.ko) modules/arch/x86/acpi/acpi.ko

OBJ_modules/arch/x86/acpi/acpi.ko:=modules/arch/x86/acpi/main.o modules/arch/x86/acpi/driver.o

modules/arch/x86/acpi/acpi.ko: lib/memory.o lib/string.o lib/arch/x86/io.o $(OBJ_modules/arch/x86/acpi/acpi.ko)
	$(LD) $(LDFLAGS) -o $@ $^
