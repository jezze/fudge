modules-arch:
	@${GCC} ${GCCFLAGS} modules/arch/x86/acpi/driver.c -o modules/arch/x86/acpi/driver.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/acpi/main.c -o modules/arch/x86/acpi/main.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/ata/bus.c -o modules/arch/x86/ata/bus.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/ata/device.c -o modules/arch/x86/ata/device.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/ata/main.c -o modules/arch/x86/ata/main.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/bga/driver.c -o modules/arch/x86/bga/driver.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/bga/main.c -o modules/arch/x86/bga/main.o
	@${ASM} ${ASMFLAGS} modules/arch/x86/cpuid/cpuid.s -o modules/arch/x86/cpuid/cpuid.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/cpuid/driver.c -o modules/arch/x86/cpuid/driver.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/cpuid/main.c -o modules/arch/x86/cpuid/main.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/i915/driver.c -o modules/arch/x86/i915/driver.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/i915/main.c -o modules/arch/x86/i915/main.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/pci/bus.c -o modules/arch/x86/pci/bus.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/pci/device.c -o modules/arch/x86/pci/device.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/pci/main.c -o modules/arch/x86/pci/main.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/pit/device.c -o modules/arch/x86/pit/device.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/pit/driver.c -o modules/arch/x86/pit/driver.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/pit/main.c -o modules/arch/x86/pit/main.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/ps2/kbd_device.c -o modules/arch/x86/ps2/kbd_device.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/ps2/kbd_driver.c -o modules/arch/x86/ps2/kbd_driver.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/ps2/main.c -o modules/arch/x86/ps2/main.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/ps2/mouse_device.c -o modules/arch/x86/ps2/mouse_device.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/ps2/mouse_driver.c -o modules/arch/x86/ps2/mouse_driver.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/rtc/device.c -o modules/arch/x86/rtc/device.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/rtc/main.c -o modules/arch/x86/rtc/main.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/rtl8139/driver.c -o modules/arch/x86/rtl8139/driver.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/rtl8139/main.c -o modules/arch/x86/rtl8139/main.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/smp/driver.c -o modules/arch/x86/smp/driver.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/smp/main.c -o modules/arch/x86/smp/main.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/uart/device.c -o modules/arch/x86/uart/device.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/uart/driver.c -o modules/arch/x86/uart/driver.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/uart/main.c -o modules/arch/x86/uart/main.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/vga/driver.c -o modules/arch/x86/vga/driver.o
	@${GCC} ${GCCFLAGS} modules/arch/x86/vga/main.c -o modules/arch/x86/vga/main.o
	@${LD} ${LDFLAGS} modules/arch/x86/acpi/driver.o modules/arch/x86/acpi/main.o -o modules/arch/x86/acpi/acpi.ko
	@${LD} ${LDFLAGS} modules/arch/x86/ata/bus.o modules/arch/x86/ata/device.o modules/arch/x86/ata/main.o -o modules/arch/x86/ata/ata.ko
	@${LD} ${LDFLAGS} modules/arch/x86/bga/driver.o modules/arch/x86/bga/main.o -o modules/arch/x86/bga/bga.ko
	@${LD} ${LDFLAGS} modules/arch/x86/cpuid/cpuid.o modules/arch/x86/cpuid/driver.o modules/arch/x86/cpuid/main.o -o modules/arch/x86/cpuid/cpuid.ko
	@${LD} ${LDFLAGS} modules/arch/x86/i915/driver.o modules/arch/x86/i915/main.o -o modules/arch/x86/i915/i915.ko
	@${LD} ${LDFLAGS} modules/arch/x86/pci/bus.o modules/arch/x86/pci/device.o modules/arch/x86/pci/main.o -o modules/arch/x86/pci/pci.ko
	@${LD} ${LDFLAGS} modules/arch/x86/pit/device.o modules/arch/x86/pit/driver.o modules/arch/x86/pit/main.o -o modules/arch/x86/pit/pit.ko
	@${LD} ${LDFLAGS} modules/arch/x86/ps2/kbd_device.o modules/arch/x86/ps2/kbd_driver.o modules/arch/x86/ps2/main.o modules/arch/x86/ps2/mouse_device.o modules/arch/x86/ps2/mouse_driver.o -o modules/arch/x86/ps2/ps2.ko
	@${LD} ${LDFLAGS} modules/arch/x86/rtc/device.o modules/arch/x86/rtc/main.o -o modules/arch/x86/rtc/rtc.ko
	@${LD} ${LDFLAGS} modules/arch/x86/rtl8139/driver.o modules/arch/x86/rtl8139/main.o -o modules/arch/x86/rtl8139/rtl8139.ko
	@${LD} ${LDFLAGS} modules/arch/x86/smp/driver.o modules/arch/x86/smp/main.o -o modules/arch/x86/smp/smp.ko
	@${LD} ${LDFLAGS} modules/arch/x86/uart/driver.o modules/arch/x86/uart/device.o modules/arch/x86/uart/main.o -o modules/arch/x86/uart/uart.ko
	@${LD} ${LDFLAGS} modules/arch/x86/vga/driver.o modules/arch/x86/vga/main.o -o modules/arch/x86/vga/vga.ko

modules-arch-clean:
	@rm -f modules/arch/x86/*/*.o
	@rm -f modules/arch/x86/*/*.ko

