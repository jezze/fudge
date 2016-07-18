MOD+=\
    $(DIR_SRC)/modules/arch/x86/acpi/acpi.ko \
    $(DIR_SRC)/modules/arch/x86/ahci/ahci.ko \
    $(DIR_SRC)/modules/arch/x86/apic/apic.ko \
    $(DIR_SRC)/modules/arch/x86/ata/ata.ko.0 \
    $(DIR_SRC)/modules/arch/x86/atapi/atapi.ko.0 \
    $(DIR_SRC)/modules/arch/x86/bga/bga.ko.0 \
    $(DIR_SRC)/modules/arch/x86/cpuid/cpuid.ko \
    $(DIR_SRC)/modules/arch/x86/ehci/ehci.ko.0 \
    $(DIR_SRC)/modules/arch/x86/fpu/fpu.ko \
    $(DIR_SRC)/modules/arch/x86/i915/i915.ko.0 \
    $(DIR_SRC)/modules/arch/x86/ide/ide.ko \
    $(DIR_SRC)/modules/arch/x86/io/io.ko \
    $(DIR_SRC)/modules/arch/x86/msr/msr.ko \
    $(DIR_SRC)/modules/arch/x86/ohci/ohci.ko.0 \
    $(DIR_SRC)/modules/arch/x86/pci/pci.ko \
    $(DIR_SRC)/modules/arch/x86/pic/pic.ko \
    $(DIR_SRC)/modules/arch/x86/pit/pit.ko.0 \
    $(DIR_SRC)/modules/arch/x86/platform/platform.ko \
    $(DIR_SRC)/modules/arch/x86/ps2/ps2.ko \
    $(DIR_SRC)/modules/arch/x86/ps2keyboard/ps2keyboard.ko.0 \
    $(DIR_SRC)/modules/arch/x86/ps2mouse/ps2mouse.ko.0 \
    $(DIR_SRC)/modules/arch/x86/rtc/rtc.ko.0 \
    $(DIR_SRC)/modules/arch/x86/rtl8139/rtl8139.ko.0 \
    $(DIR_SRC)/modules/arch/x86/smp/smp.ko \
    $(DIR_SRC)/modules/arch/x86/uart/uart.ko.0 \
    $(DIR_SRC)/modules/arch/x86/uhci/uhci.ko.0 \
    $(DIR_SRC)/modules/arch/x86/vga/vga.ko.0 \
    $(DIR_SRC)/modules/arch/x86/virtio/virtio.ko.0 \

OBJ_MOD+=\
    $(DIR_SRC)/modules/arch/x86/acpi/main.o \
    $(DIR_SRC)/modules/arch/x86/ahci/main.o \
    $(DIR_SRC)/modules/arch/x86/apic/main.o $(DIR_SRC)/modules/arch/x86/apic/apic.o \
    $(DIR_SRC)/modules/arch/x86/ata/main.o \
    $(DIR_SRC)/modules/arch/x86/atapi/main.o \
    $(DIR_SRC)/modules/arch/x86/bga/main.o \
    $(DIR_SRC)/modules/arch/x86/cpuid/main.o $(DIR_SRC)/modules/arch/x86/cpuid/cpuid.o \
    $(DIR_SRC)/modules/arch/x86/ehci/main.o \
    $(DIR_SRC)/modules/arch/x86/fpu/main.o $(DIR_SRC)/modules/arch/x86/fpu/fpu.o \
    $(DIR_SRC)/modules/arch/x86/i915/main.o \
    $(DIR_SRC)/modules/arch/x86/ide/main.o \
    $(DIR_SRC)/modules/arch/x86/io/io.o \
    $(DIR_SRC)/modules/arch/x86/msr/main.o $(DIR_SRC)/modules/arch/x86/msr/msr.o \
    $(DIR_SRC)/modules/arch/x86/ohci/main.o \
    $(DIR_SRC)/modules/arch/x86/pci/main.o \
    $(DIR_SRC)/modules/arch/x86/pic/main.o $(DIR_SRC)/modules/arch/x86/pic/pic.o \
    $(DIR_SRC)/modules/arch/x86/pit/main.o \
    $(DIR_SRC)/modules/arch/x86/platform/main.o \
    $(DIR_SRC)/modules/arch/x86/ps2/main.o \
    $(DIR_SRC)/modules/arch/x86/ps2keyboard/main.o \
    $(DIR_SRC)/modules/arch/x86/ps2mouse/main.o \
    $(DIR_SRC)/modules/arch/x86/rtc/main.o \
    $(DIR_SRC)/modules/arch/x86/rtl8139/main.o \
    $(DIR_SRC)/modules/arch/x86/smp/main.o \
    $(DIR_SRC)/modules/arch/x86/uart/main.o \
    $(DIR_SRC)/modules/arch/x86/uhci/main.o \
    $(DIR_SRC)/modules/arch/x86/vga/main.o $(DIR_SRC)/modules/arch/x86/vga/registers.o \
    $(DIR_SRC)/modules/arch/x86/virtio/main.o \

$(DIR_SRC)/modules/arch/x86/acpi/acpi.ko: $(DIR_SRC)/modules/arch/x86/acpi/main.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/ahci/ahci.ko: $(DIR_SRC)/modules/arch/x86/ahci/main.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/apic/apic.ko: $(DIR_SRC)/modules/arch/x86/apic/main.o $(DIR_SRC)/modules/arch/x86/apic/apic.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/ata/ata.ko.0: $(DIR_SRC)/modules/arch/x86/ata/main.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/atapi/atapi.ko.0: $(DIR_SRC)/modules/arch/x86/atapi/main.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/bga/bga.ko.0: $(DIR_SRC)/modules/arch/x86/bga/main.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/cpuid/cpuid.ko: $(DIR_SRC)/modules/arch/x86/cpuid/main.o $(DIR_SRC)/modules/arch/x86/cpuid/cpuid.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/ehci/ehci.ko.0: $(DIR_SRC)/modules/arch/x86/ehci/main.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/fpu/fpu.ko: $(DIR_SRC)/modules/arch/x86/fpu/main.o $(DIR_SRC)/modules/arch/x86/fpu/fpu.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/i915/i915.ko.0: $(DIR_SRC)/modules/arch/x86/i915/main.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/ide/ide.ko: $(DIR_SRC)/modules/arch/x86/ide/main.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/io/io.ko: $(DIR_SRC)/modules/arch/x86/io/io.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/msr/msr.ko: $(DIR_SRC)/modules/arch/x86/msr/main.o $(DIR_SRC)/modules/arch/x86/msr/msr.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/ohci/ohci.ko.0: $(DIR_SRC)/modules/arch/x86/ohci/main.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/pci/pci.ko: $(DIR_SRC)/modules/arch/x86/pci/main.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/pic/pic.ko: $(DIR_SRC)/modules/arch/x86/pic/main.o $(DIR_SRC)/modules/arch/x86/pic/pic.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/pit/pit.ko.0: $(DIR_SRC)/modules/arch/x86/pit/main.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/platform/platform.ko: $(DIR_SRC)/modules/arch/x86/platform/main.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/ps2/ps2.ko: $(DIR_SRC)/modules/arch/x86/ps2/main.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/ps2keyboard/ps2keyboard.ko.0: $(DIR_SRC)/modules/arch/x86/ps2keyboard/main.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/ps2mouse/ps2mouse.ko.0: $(DIR_SRC)/modules/arch/x86/ps2mouse/main.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/rtc/rtc.ko.0: $(DIR_SRC)/modules/arch/x86/rtc/main.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/rtl8139/rtl8139.ko.0: $(DIR_SRC)/modules/arch/x86/rtl8139/main.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/smp/smp.ko: $(DIR_SRC)/modules/arch/x86/smp/main.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/uart/uart.ko.0: $(DIR_SRC)/modules/arch/x86/uart/main.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/uhci/uhci.ko.0: $(DIR_SRC)/modules/arch/x86/uhci/main.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/vga/vga.ko.0: $(DIR_SRC)/modules/arch/x86/vga/main.o $(DIR_SRC)/modules/arch/x86/vga/registers.o $(DIR_SRC)/fudge/fudge.a
$(DIR_SRC)/modules/arch/x86/virtio/virtio.ko.0: $(DIR_SRC)/modules/arch/x86/virtio/main.o $(DIR_SRC)/fudge/fudge.a
