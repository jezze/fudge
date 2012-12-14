MODULES_ACPI=modules/arch/x86/acpi/acpi.ko
MODULES_APIC=modules/arch/x86/apic/apic.ko
MODULES_ATA=modules/arch/x86/ata/ata.ko
MODULES_BGA=modules/arch/x86/bga/bga.ko
MODULES_CPUID=modules/arch/x86/cpuid/cpuid.ko
MODULES_FPU=modules/arch/x86/fpu/fpu.ko
MODULES_I915=modules/arch/x86/i915/i915.ko
MODULES_IO=modules/arch/x86/io/io.ko
MODULES_MSR=modules/arch/x86/msr/msr.ko
MODULES_MULTI=modules/arch/x86/multi/multi.ko
MODULES_PCI=modules/arch/x86/pci/pci.ko
MODULES_PIC=modules/arch/x86/pic/pic.ko
MODULES_PIT=modules/arch/x86/pit/pit.ko
MODULES_PS2=modules/arch/x86/ps2/ps2.ko
MODULES_RTC=modules/arch/x86/rtc/rtc.ko
MODULES_RTL8139=modules/arch/x86/rtl8139/rtl8139.ko
MODULES_SMP=modules/arch/x86/smp/smp.ko
MODULES_UART=modules/arch/x86/uart/uart.ko
MODULES_VGA=modules/arch/x86/vga/vga.ko

MODULES_OBJECTS_ACPI=modules/arch/x86/acpi/main.o
MODULES_OBJECTS_APIC=modules/arch/x86/apic/main.o
MODULES_OBJECTS_ATA=modules/arch/x86/ata/main.o modules/arch/x86/ata/bus.o modules/arch/x86/ata/device.o modules/arch/x86/ata/driver.o
MODULES_OBJECTS_BGA=modules/arch/x86/bga/main.o modules/arch/x86/bga/driver.o
MODULES_OBJECTS_CPUID=modules/arch/x86/cpuid/main.o modules/arch/x86/cpuid/cpuid.o
MODULES_OBJECTS_FPU=modules/arch/x86/fpu/main.o modules/arch/x86/fpu/fpu.o
MODULES_OBJECTS_I915=modules/arch/x86/i915/main.o modules/arch/x86/i915/driver.o
MODULES_OBJECTS_IO=modules/arch/x86/io/main.o modules/arch/x86/io/io.o
MODULES_OBJECTS_MSR=modules/arch/x86/msr/main.o modules/arch/x86/msr/msr.o
MODULES_OBJECTS_MULTI=modules/arch/x86/multi/main.o
MODULES_OBJECTS_PCI=modules/arch/x86/pci/main.o modules/arch/x86/pci/bus.o modules/arch/x86/pci/device.o
MODULES_OBJECTS_PIC=modules/arch/x86/pic/main.o modules/arch/x86/pic/pic.o
MODULES_OBJECTS_PIT=modules/arch/x86/pit/main.o modules/arch/x86/pit/driver.o modules/arch/x86/pit/device.o
MODULES_OBJECTS_PS2=modules/arch/x86/ps2/main.o modules/arch/x86/ps2/device.o modules/arch/x86/ps2/kbd_driver.o modules/arch/x86/ps2/mouse_driver.o
MODULES_OBJECTS_RTC=modules/arch/x86/rtc/main.o modules/arch/x86/rtc/device.o
MODULES_OBJECTS_RTL8139=modules/arch/x86/rtl8139/main.o modules/arch/x86/rtl8139/driver.o
MODULES_OBJECTS_SMP=modules/arch/x86/smp/main.o
MODULES_OBJECTS_UART=modules/arch/x86/uart/main.o modules/arch/x86/uart/driver.o modules/arch/x86/uart/device.o
MODULES_OBJECTS_VGA=modules/arch/x86/vga/main.o modules/arch/x86/vga/driver.o

MODULES+=$(MODULES_ACPI) $(MODULES_APIC) $(MODULES_ATA) $(MODULES_BGA) $(MODULES_CPUID) $(MODULES_FPU) $(MODULES_I915) $(MODULES_IO) $(MODULES_MSR) $(MODULES_MULTI) $(MODULES_PCI) $(MODULES_PIC) $(MODULES_PIT) $(MODULES_PS2) $(MODULES_RTC) $(MODULES_RTL8139) $(MODULES_SMP) $(MODULES_UART) $(MODULES_VGA)
MODULES_OBJECTS+=$(MODULES_OBJECTS_ACPI) $(MODULES_OBJECTS_APIC) $(MODULES_OBJECTS_ATA) $(MODULES_OBJECTS_BGA) $(MODULES_OBJECTS_CPUID) $(MODULES_OBJECTS_FPU) $(MODULES_OBJECTS_I915) $(MODULES_OBJECTS_IO) $(MODULES_OBJECTS_MSR) $(MODULES_OBJECTS_MULTI) $(MODULES_OBJECTS_PCI) $(MODULES_OBJECTS_PIC) $(MODULES_OBJECTS_PIT) $(MODULES_OBJECTS_PS2) $(MODULES_OBJECTS_RTC) $(MODULES_OBJECTS_RTL8139) $(MODULES_OBJECTS_SMP) $(MODULES_OBJECTS_UART) $(MODULES_OBJECTS_VGA)

$(MODULES_ACPI): $(MODULES_OBJECTS_ACPI) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_APIC): $(MODULES_OBJECTS_APIC) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_ATA): $(MODULES_OBJECTS_ATA) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_BGA): $(MODULES_OBJECTS_BGA) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_CPUID): $(MODULES_OBJECTS_CPUID) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_FPU): $(MODULES_OBJECTS_FPU) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_I915): $(MODULES_OBJECTS_I915) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_IO): $(MODULES_OBJECTS_IO) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_MSR): $(MODULES_OBJECTS_MSR) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_MULTI): $(MODULES_OBJECTS_MULTI) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_PCI): $(MODULES_OBJECTS_PCI) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_PIC): $(MODULES_OBJECTS_PIC) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_PIT): $(MODULES_OBJECTS_PIT) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_PS2): $(MODULES_OBJECTS_PS2) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_RTC): $(MODULES_OBJECTS_RTC) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_RTL8139): $(MODULES_OBJECTS_RTL8139) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_SMP): $(MODULES_OBJECTS_SMP) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_UART): $(MODULES_OBJECTS_UART) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

$(MODULES_VGA): $(MODULES_OBJECTS_VGA) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
