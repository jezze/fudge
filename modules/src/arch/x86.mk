MODULES+=modules/src/arch/x86/acpi/acpi.ko
MODULES+=modules/src/arch/x86/apic/apic.ko
MODULES+=modules/src/arch/x86/ata/ata.ko
MODULES+=modules/src/arch/x86/bga/bga.ko
MODULES+=modules/src/arch/x86/cpuid/cpuid.ko
MODULES+=modules/src/arch/x86/fpu/fpu.ko
MODULES+=modules/src/arch/x86/i915/i915.ko
MODULES+=modules/src/arch/x86/msr/msr.ko
MODULES+=modules/src/arch/x86/pci/pci.ko
MODULES+=modules/src/arch/x86/pit/pit.ko
MODULES+=modules/src/arch/x86/ps2/ps2.ko
MODULES+=modules/src/arch/x86/rtc/rtc.ko
MODULES+=modules/src/arch/x86/rtl8139/rtl8139.ko
MODULES+=modules/src/arch/x86/smp/smp.ko
MODULES+=modules/src/arch/x86/uart/uart.ko
MODULES+=modules/src/arch/x86/vga/vga.ko

MODULESOBJ+=modules/src/arch/x86/acpi/main.o modules/src/arch/x86/acpi/driver.o
MODULESOBJ+=modules/src/arch/x86/apic/main.o modules/src/arch/x86/apic/driver.o
MODULESOBJ+=modules/src/arch/x86/ata/main.o modules/src/arch/x86/ata/bus.o modules/src/arch/x86/ata/device.o
MODULESOBJ+=modules/src/arch/x86/bga/main.o modules/src/arch/x86/bga/driver.o
MODULESOBJ+=modules/src/arch/x86/cpuid/main.o modules/src/arch/x86/cpuid/driver.o modules/src/arch/x86/cpuid/cpuid.o
MODULESOBJ+=modules/src/arch/x86/fpu/main.o modules/src/arch/x86/fpu/driver.o modules/src/arch/x86/fpu/fpu.o
MODULESOBJ+=modules/src/arch/x86/i915/main.o modules/src/arch/x86/i915/driver.o
MODULESOBJ+=modules/src/arch/x86/msr/main.o modules/src/arch/x86/msr/driver.o modules/src/arch/x86/msr/msr.o
MODULESOBJ+=modules/src/arch/x86/pci/main.o modules/src/arch/x86/pci/bus.o modules/src/arch/x86/pci/device.o
MODULESOBJ+=modules/src/arch/x86/pit/main.o modules/src/arch/x86/pit/driver.o modules/src/arch/x86/pit/device.o
MODULESOBJ+=modules/src/arch/x86/ps2/main.o modules/src/arch/x86/ps2/kbd_driver.o modules/src/arch/x86/ps2/kbd_device.o modules/src/arch/x86/ps2/mouse_driver.o modules/src/arch/x86/ps2/mouse_device.o
MODULESOBJ+=modules/src/arch/x86/rtc/main.o modules/src/arch/x86/rtc/device.o
MODULESOBJ+=modules/src/arch/x86/rtl8139/main.o modules/src/arch/x86/rtl8139/driver.o
MODULESOBJ+=modules/src/arch/x86/smp/main.o modules/src/arch/x86/smp/driver.o
MODULESOBJ+=modules/src/arch/x86/uart/main.o modules/src/arch/x86/uart/driver.o modules/src/arch/x86/uart/device.o
MODULESOBJ+=modules/src/arch/x86/vga/main.o modules/src/arch/x86/vga/driver.o

modules/src/arch/x86/acpi/acpi.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/acpi/main.o modules/src/arch/x86/acpi/driver.o
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/apic/apic.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/apic/main.o modules/src/arch/x86/apic/driver.o
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/ata/ata.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/ata/main.o modules/src/arch/x86/ata/bus.o modules/src/arch/x86/ata/device.o
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/bga/bga.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/bga/main.o modules/src/arch/x86/bga/driver.o
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/cpuid/cpuid.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/cpuid/main.o modules/src/arch/x86/cpuid/driver.o modules/src/arch/x86/cpuid/cpuid.o
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/fpu/fpu.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/fpu/main.o modules/src/arch/x86/fpu/driver.o modules/src/arch/x86/fpu/fpu.o
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/i915/i915.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/i915/main.o modules/src/arch/x86/i915/driver.o
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/msr/msr.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/msr/main.o modules/src/arch/x86/msr/driver.o modules/src/arch/x86/msr/msr.o
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/pci/pci.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/pci/main.o modules/src/arch/x86/pci/bus.o modules/src/arch/x86/pci/device.o
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/pit/pit.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/pit/main.o modules/src/arch/x86/pit/driver.o modules/src/arch/x86/pit/device.o
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/ps2/ps2.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/ps2/main.o modules/src/arch/x86/ps2/kbd_driver.o modules/src/arch/x86/ps2/kbd_device.o modules/src/arch/x86/ps2/mouse_driver.o modules/src/arch/x86/ps2/mouse_device.o
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/rtc/rtc.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/rtc/main.o modules/src/arch/x86/rtc/device.o
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/rtl8139/rtl8139.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/rtl8139/main.o modules/src/arch/x86/rtl8139/driver.o
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/smp/smp.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/smp/main.o modules/src/arch/x86/smp/driver.o
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/uart/uart.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/uart/main.o modules/src/arch/x86/uart/driver.o modules/src/arch/x86/uart/device.o
	$(LD) $(LDFLAGS) -o $@ $^

modules/src/arch/x86/vga/vga.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/vga/main.o modules/src/arch/x86/vga/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
