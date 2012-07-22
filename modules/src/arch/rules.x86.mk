MODULE_ACPI=modules/src/arch/x86/acpi/acpi.ko
MODULE_APIC=modules/src/arch/x86/apic/apic.ko
MODULE_ATA=modules/src/arch/x86/ata/ata.ko
MODULE_BGA=modules/src/arch/x86/bga/bga.ko
MODULE_CPUID=modules/src/arch/x86/cpuid/cpuid.ko
MODULE_FPU=modules/src/arch/x86/fpu/fpu.ko
MODULE_I915=modules/src/arch/x86/i915/i915.ko
MODULE_MSR=modules/src/arch/x86/msr/msr.ko
MODULE_PCI=modules/src/arch/x86/pci/pci.ko
MODULE_PIT=modules/src/arch/x86/pit/pit.ko
MODULE_PS2=modules/src/arch/x86/ps2/ps2.ko
MODULE_RTC=modules/src/arch/x86/rtc/rtc.ko
MODULE_RTL8139=modules/src/arch/x86/rtl8139/rtl8139.ko
MODULE_SMP=modules/src/arch/x86/smp/smp.ko
MODULE_UART=modules/src/arch/x86/uart/uart.ko
MODULE_VGA=modules/src/arch/x86/vga/vga.ko

MODULES+=$(MODULE_ACPI) $(MODULE_APIC) $(MODULE_ATA) $(MODULE_BGA) $(MODULE_CPUID) $(MODULE_FPU) $(MODULE_I915) $(MODULE_MSR) $(MODULE_PCI) $(MODULE_PIT) $(MODULE_PS2) $(MODULE_RTC) $(MODULE_RTL8139) $(MODULE_SMP) $(MODULE_UART) $(MODULE_VGA)

OBJECTS_ACPI=modules/src/arch/x86/acpi/main.o modules/src/arch/x86/acpi/driver.o
OBJECTS_APIC=modules/src/arch/x86/apic/main.o modules/src/arch/x86/apic/driver.o
OBJECTS_ATA=modules/src/arch/x86/ata/main.o modules/src/arch/x86/ata/bus.o modules/src/arch/x86/ata/device.o
OBJECTS_BGA=modules/src/arch/x86/bga/main.o modules/src/arch/x86/bga/driver.o
OBJECTS_CPUID=modules/src/arch/x86/cpuid/main.o modules/src/arch/x86/cpuid/driver.o modules/src/arch/x86/cpuid/cpuid.o
OBJECTS_FPU=modules/src/arch/x86/fpu/main.o modules/src/arch/x86/fpu/driver.o modules/src/arch/x86/fpu/fpu.o
OBJECTS_I915=modules/src/arch/x86/i915/main.o modules/src/arch/x86/i915/driver.o
OBJECTS_MSR=modules/src/arch/x86/msr/main.o modules/src/arch/x86/msr/driver.o modules/src/arch/x86/msr/msr.o
OBJECTS_PCI=modules/src/arch/x86/pci/main.o modules/src/arch/x86/pci/bus.o modules/src/arch/x86/pci/device.o
OBJECTS_PIT=modules/src/arch/x86/pit/main.o modules/src/arch/x86/pit/driver.o modules/src/arch/x86/pit/device.o
OBJECTS_PS2=modules/src/arch/x86/ps2/main.o modules/src/arch/x86/ps2/kbd_driver.o modules/src/arch/x86/ps2/kbd_device.o modules/src/arch/x86/ps2/mouse_driver.o modules/src/arch/x86/ps2/mouse_device.o
OBJECTS_RTC=modules/src/arch/x86/rtc/main.o modules/src/arch/x86/rtc/device.o
OBJECTS_RTL8139=modules/src/arch/x86/rtl8139/main.o modules/src/arch/x86/rtl8139/driver.o
OBJECTS_SMP=modules/src/arch/x86/smp/main.o modules/src/arch/x86/smp/driver.o
OBJECTS_UART=modules/src/arch/x86/uart/main.o modules/src/arch/x86/uart/driver.o modules/src/arch/x86/uart/device.o
OBJECTS_VGA=modules/src/arch/x86/vga/main.o modules/src/arch/x86/vga/driver.o

OBJECTS+=$(OBJECTS_ACPI) $(OBJECTS_APIC) $(OBJECTS_ATA) $(OBJECTS_BGA) $(OBJECTS_CPUID) $(OBJECTS_FPU) $(OBJECTS_I915) $(OBJECTS_MSR) $(OBJECTS_PCI) $(OBJECTS_PIT) $(OBJECTS_PS2) $(OBJECTS_RTC) $(OBJECTS_RTL8139) $(OBJECTS_SMP) $(OBJECTS_UART) $(OBJECTS_VGA)

modules/src/arch/x86/acpi/acpi.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o $(OBJECTS_ACPI)
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/apic/apic.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o $(OBJECTS_APIC) 
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/ata/ata.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o $(OBJECTS_ATA)
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/bga/bga.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o $(OBJECTS_BGA)
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/cpuid/cpuid.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o $(OBJECTS_CPUID)
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/fpu/fpu.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o $(OBJECTS_FPU)
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/i915/i915.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o $(OBJECTS_I915)
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/msr/msr.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o $(OBJECTS_MSR)
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/pci/pci.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o $(OBJECTS_PCI)
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/pit/pit.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o $(OBJECTS_PIT)
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/ps2/ps2.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o $(OBJECTS_PS2)
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/rtc/rtc.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o $(OBJECTS_RTC)
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/rtl8139/rtl8139.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o $(OBJECTS_RTL8139)
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/smp/smp.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o $(OBJECTS_SMP)
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/uart/uart.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o $(OBJECTS_UART)
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/vga/vga.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o $(OBJECTS_VGA)
	$(LD) $(LDFLAGS) -o $@ $^
