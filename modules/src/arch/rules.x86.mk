MODULES_ACPI=modules/src/arch/x86/acpi/acpi.ko
MODULES_APIC=modules/src/arch/x86/apic/apic.ko
MODULES_ATA=modules/src/arch/x86/ata/ata.ko
MODULES_BGA=modules/src/arch/x86/bga/bga.ko
MODULES_CPUID=modules/src/arch/x86/cpuid/cpuid.ko
MODULES_FPU=modules/src/arch/x86/fpu/fpu.ko
MODULES_I915=modules/src/arch/x86/i915/i915.ko
MODULES_IO=modules/src/arch/x86/io/io.ko
MODULES_MSR=modules/src/arch/x86/msr/msr.ko
MODULES_PCI=modules/src/arch/x86/pci/pci.ko
MODULES_PIT=modules/src/arch/x86/pit/pit.ko
MODULES_PS2=modules/src/arch/x86/ps2/ps2.ko
MODULES_RTC=modules/src/arch/x86/rtc/rtc.ko
MODULES_RTL8139=modules/src/arch/x86/rtl8139/rtl8139.ko
MODULES_SMP=modules/src/arch/x86/smp/smp.ko
MODULES_UART=modules/src/arch/x86/uart/uart.ko
MODULES_VGA=modules/src/arch/x86/vga/vga.ko

MODULES_OBJECTS_ACPI=modules/src/arch/x86/acpi/main.o modules/src/arch/x86/acpi/driver.o
MODULES_OBJECTS_APIC=modules/src/arch/x86/apic/main.o
MODULES_OBJECTS_ATA=modules/src/arch/x86/ata/main.o modules/src/arch/x86/ata/bus.o modules/src/arch/x86/ata/device.o
MODULES_OBJECTS_BGA=modules/src/arch/x86/bga/main.o modules/src/arch/x86/bga/driver.o
MODULES_OBJECTS_CPUID=modules/src/arch/x86/cpuid/main.o modules/src/arch/x86/cpuid/cpuid.o
MODULES_OBJECTS_FPU=modules/src/arch/x86/fpu/main.o modules/src/arch/x86/fpu/fpu.o
MODULES_OBJECTS_I915=modules/src/arch/x86/i915/main.o modules/src/arch/x86/i915/driver.o
MODULES_OBJECTS_IO=modules/src/arch/x86/io/main.o modules/src/arch/x86/io/io.o
MODULES_OBJECTS_MSR=modules/src/arch/x86/msr/main.o modules/src/arch/x86/msr/msr.o
MODULES_OBJECTS_PCI=modules/src/arch/x86/pci/main.o modules/src/arch/x86/pci/bus.o modules/src/arch/x86/pci/device.o
MODULES_OBJECTS_PIT=modules/src/arch/x86/pit/main.o modules/src/arch/x86/pit/driver.o modules/src/arch/x86/pit/device.o
MODULES_OBJECTS_PS2=modules/src/arch/x86/ps2/main.o modules/src/arch/x86/ps2/kbd_driver.o modules/src/arch/x86/ps2/kbd_device.o modules/src/arch/x86/ps2/mouse_driver.o modules/src/arch/x86/ps2/mouse_device.o
MODULES_OBJECTS_RTC=modules/src/arch/x86/rtc/main.o modules/src/arch/x86/rtc/device.o
MODULES_OBJECTS_RTL8139=modules/src/arch/x86/rtl8139/main.o modules/src/arch/x86/rtl8139/driver.o
MODULES_OBJECTS_SMP=modules/src/arch/x86/smp/main.o
MODULES_OBJECTS_UART=modules/src/arch/x86/uart/main.o modules/src/arch/x86/uart/driver.o modules/src/arch/x86/uart/device.o
MODULES_OBJECTS_VGA=modules/src/arch/x86/vga/main.o modules/src/arch/x86/vga/driver.o

MODULES+=$(MODULES_ACPI) $(MODULES_APIC) $(MODULES_ATA) $(MODULES_BGA) $(MODULES_CPUID) $(MODULES_FPU) $(MODULES_I915) $(MODULES_IO) $(MODULES_MSR) $(MODULES_PCI) $(MODULES_PIT) $(MODULES_PS2) $(MODULES_RTC) $(MODULES_RTL8139) $(MODULES_SMP) $(MODULES_UART) $(MODULES_VGA)
MODULES_OBJECTS+=$(MODULES_OBJECTS_ACPI) $(MODULES_OBJECTS_APIC) $(MODULES_OBJECTS_ATA) $(MODULES_OBJECTS_BGA) $(MODULES_OBJECTS_CPUID) $(MODULES_OBJECTS_FPU) $(MODULES_OBJECTS_I915) $(MODULES_OBJECTS_IO) $(MODULES_OBJECTS_MSR) $(MODULES_OBJECTS_PCI) $(MODULES_OBJECTS_PIT) $(MODULES_OBJECTS_PS2) $(MODULES_OBJECTS_RTC) $(MODULES_OBJECTS_RTL8139) $(MODULES_OBJECTS_SMP) $(MODULES_OBJECTS_UART) $(MODULES_OBJECTS_VGA)

$(MODULES_ACPI): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_ACPI)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_APIC): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_APIC)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_ATA): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_ATA)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_BGA): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_BGA)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_CPUID): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_CPUID)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_FPU): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_FPU)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_I915): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_I915)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_IO): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_IO)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_MSR): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_MSR)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_PCI): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_PCI)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_PIT): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_PIT)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_PS2): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_PS2)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_RTC): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_RTC)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_RTL8139): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_RTL8139)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_SMP): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_SMP)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_UART): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_UART)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_VGA): lib/src/memory.o lib/src/string.o $(MODULES_OBJECTS_VGA)
	$(LD) $(LDFLAGS) -o $@ $^
