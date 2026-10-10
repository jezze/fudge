M:=\
    $(DIR_SRC)/modules/audio.ko \

N:=\
    $(DIR_SRC)/modules/audio.ko.map \

O:=\
    $(DIR_SRC)/modules/audio.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/base.ko \

N:=\
    $(DIR_SRC)/modules/base.ko.map \

O:=\
    $(DIR_SRC)/modules/bus.o \
    $(DIR_SRC)/modules/driver.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/block.ko \

N:=\
    $(DIR_SRC)/modules/block.ko.map \

O:=\
    $(DIR_SRC)/modules/block.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/clock.ko \

N:=\
    $(DIR_SRC)/modules/clock.ko.map \

O:=\
    $(DIR_SRC)/modules/clock.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/console.ko \

N:=\
    $(DIR_SRC)/modules/console.ko.map \

O:=\
    $(DIR_SRC)/modules/console.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/ethernet.ko \

N:=\
    $(DIR_SRC)/modules/ethernet.ko.map \

O:=\
    $(DIR_SRC)/modules/ethernet.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/net/net.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/info.ko \

N:=\
    $(DIR_SRC)/modules/info.ko.map \

O:=\
    $(DIR_SRC)/modules/info.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/keyboard.ko \

N:=\
    $(DIR_SRC)/modules/keyboard.ko.map \

O:=\
    $(DIR_SRC)/modules/keyboard.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/log.ko \

N:=\
    $(DIR_SRC)/modules/log.ko.map \

O:=\
    $(DIR_SRC)/modules/log.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/mouse.ko \

N:=\
    $(DIR_SRC)/modules/mouse.ko.map \

O:=\
    $(DIR_SRC)/modules/mouse.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/timer.ko \

N:=\
    $(DIR_SRC)/modules/timer.ko.map \

O:=\
    $(DIR_SRC)/modules/timer.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/video.ko \

N:=\
    $(DIR_SRC)/modules/video.ko.map \

O:=\
    $(DIR_SRC)/modules/video.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
ifeq ($(ARCH),x86)
include $(DIR_SRC)/modules/64bit/rules.mk
include $(DIR_SRC)/modules/acpi/x86/rules.mk
include $(DIR_SRC)/modules/ahci/rules.mk
include $(DIR_SRC)/modules/apic/x86/rules.mk
include $(DIR_SRC)/modules/ata/x86/rules.mk
include $(DIR_SRC)/modules/atapi/rules.mk
include $(DIR_SRC)/modules/bga/x86/rules.mk
include $(DIR_SRC)/modules/cpuid/x86/rules.mk
include $(DIR_SRC)/modules/fpu/rules.mk
include $(DIR_SRC)/modules/gop/x86/rules.mk
include $(DIR_SRC)/modules/i915/rules.mk
include $(DIR_SRC)/modules/ide/rules.mk
include $(DIR_SRC)/modules/io/x86/rules.mk
include $(DIR_SRC)/modules/msr/x86/rules.mk
include $(DIR_SRC)/modules/nvme/rules.mk
include $(DIR_SRC)/modules/pat/rules.mk
include $(DIR_SRC)/modules/pci/rules.mk
include $(DIR_SRC)/modules/pic/x86/rules.mk
include $(DIR_SRC)/modules/pit/rules.mk
include $(DIR_SRC)/modules/platform/rules.mk
include $(DIR_SRC)/modules/ps2/rules.mk
include $(DIR_SRC)/modules/rtc/rules.mk
include $(DIR_SRC)/modules/rtl8139/rules.mk
include $(DIR_SRC)/modules/smp/x86/rules.mk
ifeq ($(TARGET),i386-unknown-elf)
include $(DIR_SRC)/modules/syse/rules.mk
endif
include $(DIR_SRC)/modules/uart/rules.mk
include $(DIR_SRC)/modules/usb/rules.mk
include $(DIR_SRC)/modules/vbe/rules.mk
include $(DIR_SRC)/modules/vga/rules.mk
include $(DIR_SRC)/modules/virtio/rules.mk
endif
ifeq ($(ARCH),x86_64)
include $(DIR_SRC)/modules/acpi/x86_64/rules.mk
include $(DIR_SRC)/modules/ahci/rules.mk
include $(DIR_SRC)/modules/apic/x86_64/rules.mk
include $(DIR_SRC)/modules/ata/x86_64/rules.mk
include $(DIR_SRC)/modules/atapi/rules.mk
include $(DIR_SRC)/modules/bga/x86_64/rules.mk
include $(DIR_SRC)/modules/cpuid/x86_64/rules.mk
include $(DIR_SRC)/modules/gop/x86_64/rules.mk
include $(DIR_SRC)/modules/i915/rules.mk
include $(DIR_SRC)/modules/ide/rules.mk
include $(DIR_SRC)/modules/io/x86_64/rules.mk
include $(DIR_SRC)/modules/msr/x86_64/rules.mk
include $(DIR_SRC)/modules/nvme/rules.mk
include $(DIR_SRC)/modules/pat/rules.mk
include $(DIR_SRC)/modules/pci/rules.mk
include $(DIR_SRC)/modules/pic/x86_64/rules.mk
include $(DIR_SRC)/modules/pit/rules.mk
include $(DIR_SRC)/modules/platform/rules.mk
include $(DIR_SRC)/modules/ps2/rules.mk
include $(DIR_SRC)/modules/rtc/rules.mk
include $(DIR_SRC)/modules/rtl8139/rules.mk
include $(DIR_SRC)/modules/smp/x86_64/rules.mk
include $(DIR_SRC)/modules/uart/rules.mk
include $(DIR_SRC)/modules/usb/rules.mk
include $(DIR_SRC)/modules/vga/rules.mk
include $(DIR_SRC)/modules/virtio/rules.mk
endif
