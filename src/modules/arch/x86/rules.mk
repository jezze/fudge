MODULES+=\
    src/modules/arch/x86/acpi/acpi.ko \
    src/modules/arch/x86/ahci/ahci.ko \
    src/modules/arch/x86/apic/apic.ko \
    src/modules/arch/x86/ata/ata.ko.0 \
    src/modules/arch/x86/atapi/atapi.ko.0 \
    src/modules/arch/x86/bga/bga.ko.0 \
    src/modules/arch/x86/cpuid/cpuid.ko \
    src/modules/arch/x86/ehci/ehci.ko.0 \
    src/modules/arch/x86/fpu/fpu.ko \
    src/modules/arch/x86/i915/i915.ko.0 \
    src/modules/arch/x86/ide/ide.ko \
    src/modules/arch/x86/io/io.ko \
    src/modules/arch/x86/msr/msr.ko \
    src/modules/arch/x86/ohci/ohci.ko.0 \
    src/modules/arch/x86/pci/pci.ko \
    src/modules/arch/x86/pic/pic.ko \
    src/modules/arch/x86/pit/pit.ko.0 \
    src/modules/arch/x86/platform/platform.ko \
    src/modules/arch/x86/ps2/ps2.ko \
    src/modules/arch/x86/ps2keyboard/ps2keyboard.ko.0 \
    src/modules/arch/x86/ps2mouse/ps2mouse.ko.0 \
    src/modules/arch/x86/rtc/rtc.ko.0 \
    src/modules/arch/x86/rtl8139/rtl8139.ko.0 \
    src/modules/arch/x86/smp/smp.ko \
    src/modules/arch/x86/uart/uart.ko.0 \
    src/modules/arch/x86/uhci/uhci.ko.0 \
    src/modules/arch/x86/vga/vga.ko.0 \
    src/modules/arch/x86/virtio/virtio.ko.0 \

OBJ_MODULES+=\
    src/modules/arch/x86/acpi/main.o \
    src/modules/arch/x86/ahci/main.o \
    src/modules/arch/x86/apic/main.o src/modules/arch/x86/apic/apic.o \
    src/modules/arch/x86/ata/main.o \
    src/modules/arch/x86/atapi/main.o \
    src/modules/arch/x86/bga/main.o \
    src/modules/arch/x86/cpuid/main.o src/modules/arch/x86/cpuid/cpuid.o \
    src/modules/arch/x86/ehci/main.o \
    src/modules/arch/x86/fpu/main.o src/modules/arch/x86/fpu/fpu.o \
    src/modules/arch/x86/i915/main.o \
    src/modules/arch/x86/ide/main.o \
    src/modules/arch/x86/io/io.o \
    src/modules/arch/x86/msr/main.o src/modules/arch/x86/msr/msr.o \
    src/modules/arch/x86/ohci/main.o \
    src/modules/arch/x86/pci/main.o \
    src/modules/arch/x86/pic/main.o src/modules/arch/x86/pic/pic.o \
    src/modules/arch/x86/pit/main.o \
    src/modules/arch/x86/platform/main.o \
    src/modules/arch/x86/ps2/main.o \
    src/modules/arch/x86/ps2keyboard/main.o \
    src/modules/arch/x86/ps2mouse/main.o \
    src/modules/arch/x86/rtc/main.o \
    src/modules/arch/x86/rtl8139/main.o \
    src/modules/arch/x86/smp/main.o \
    src/modules/arch/x86/uart/main.o \
    src/modules/arch/x86/uhci/main.o \
    src/modules/arch/x86/vga/main.o src/modules/arch/x86/vga/registers.o \
    src/modules/arch/x86/virtio/main.o \

src/modules/arch/x86/acpi/acpi.ko: src/modules/arch/x86/acpi/main.o $(MOD_DEPS)
src/modules/arch/x86/ahci/ahci.ko: src/modules/arch/x86/ahci/main.o $(MOD_DEPS)
src/modules/arch/x86/apic/apic.ko: src/modules/arch/x86/apic/main.o src/modules/arch/x86/apic/apic.o $(MOD_DEPS)
src/modules/arch/x86/ata/ata.ko.0: src/modules/arch/x86/ata/main.o $(MOD_DEPS)
src/modules/arch/x86/atapi/atapi.ko.0: src/modules/arch/x86/atapi/main.o $(MOD_DEPS)
src/modules/arch/x86/bga/bga.ko.0: src/modules/arch/x86/bga/main.o $(MOD_DEPS)
src/modules/arch/x86/cpuid/cpuid.ko: src/modules/arch/x86/cpuid/main.o src/modules/arch/x86/cpuid/cpuid.o $(MOD_DEPS)
src/modules/arch/x86/ehci/ehci.ko.0: src/modules/arch/x86/ehci/main.o $(MOD_DEPS)
src/modules/arch/x86/fpu/fpu.ko: src/modules/arch/x86/fpu/main.o src/modules/arch/x86/fpu/fpu.o $(MOD_DEPS)
src/modules/arch/x86/i915/i915.ko.0: src/modules/arch/x86/i915/main.o $(MOD_DEPS)
src/modules/arch/x86/ide/ide.ko: src/modules/arch/x86/ide/main.o $(MOD_DEPS)
src/modules/arch/x86/io/io.ko: src/modules/arch/x86/io/io.o $(MOD_DEPS)
src/modules/arch/x86/msr/msr.ko: src/modules/arch/x86/msr/main.o src/modules/arch/x86/msr/msr.o $(MOD_DEPS)
src/modules/arch/x86/ohci/ohci.ko.0: src/modules/arch/x86/ohci/main.o $(MOD_DEPS)
src/modules/arch/x86/pci/pci.ko: src/modules/arch/x86/pci/main.o $(MOD_DEPS)
src/modules/arch/x86/pic/pic.ko: src/modules/arch/x86/pic/main.o src/modules/arch/x86/pic/pic.o $(MOD_DEPS)
src/modules/arch/x86/pit/pit.ko.0: src/modules/arch/x86/pit/main.o $(MOD_DEPS)
src/modules/arch/x86/platform/platform.ko: src/modules/arch/x86/platform/main.o $(MOD_DEPS)
src/modules/arch/x86/ps2/ps2.ko: src/modules/arch/x86/ps2/main.o $(MOD_DEPS)
src/modules/arch/x86/ps2keyboard/ps2keyboard.ko.0: src/modules/arch/x86/ps2keyboard/main.o $(MOD_DEPS)
src/modules/arch/x86/ps2mouse/ps2mouse.ko.0: src/modules/arch/x86/ps2mouse/main.o $(MOD_DEPS)
src/modules/arch/x86/rtc/rtc.ko.0: src/modules/arch/x86/rtc/main.o $(MOD_DEPS)
src/modules/arch/x86/rtl8139/rtl8139.ko.0: src/modules/arch/x86/rtl8139/main.o $(MOD_DEPS)
src/modules/arch/x86/smp/smp.ko: src/modules/arch/x86/smp/main.o $(MOD_DEPS)
src/modules/arch/x86/uart/uart.ko.0: src/modules/arch/x86/uart/main.o $(MOD_DEPS)
src/modules/arch/x86/uhci/uhci.ko.0: src/modules/arch/x86/uhci/main.o $(MOD_DEPS)
src/modules/arch/x86/vga/vga.ko.0: src/modules/arch/x86/vga/main.o src/modules/arch/x86/vga/registers.o $(MOD_DEPS)
src/modules/arch/x86/virtio/virtio.ko.0: src/modules/arch/x86/virtio/main.o $(MOD_DEPS)
