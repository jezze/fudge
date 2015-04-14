PAR:=$(PAR).x
TMP_$(PAR):=$(CUR)
CUR:=$(DIR)

DIR:=$(CUR)/acpi
include $(DIR)/rules.mk
DIR:=$(CUR)/ahci
include $(DIR)/rules.mk
DIR:=$(CUR)/apic
include $(DIR)/rules.mk
DIR:=$(CUR)/ata
include $(DIR)/rules.mk
DIR:=$(CUR)/atapi
include $(DIR)/rules.mk
DIR:=$(CUR)/bga
include $(DIR)/rules.mk
DIR:=$(CUR)/cpuid
include $(DIR)/rules.mk
DIR:=$(CUR)/ehci
include $(DIR)/rules.mk
DIR:=$(CUR)/fpu
include $(DIR)/rules.mk
DIR:=$(CUR)/i915
include $(DIR)/rules.mk
DIR:=$(CUR)/ide
include $(DIR)/rules.mk
DIR:=$(CUR)/io
include $(DIR)/rules.mk
DIR:=$(CUR)/msr
include $(DIR)/rules.mk
DIR:=$(CUR)/ohci
include $(DIR)/rules.mk
DIR:=$(CUR)/pci
include $(DIR)/rules.mk
DIR:=$(CUR)/pic
include $(DIR)/rules.mk
DIR:=$(CUR)/pit
include $(DIR)/rules.mk
DIR:=$(CUR)/platform
include $(DIR)/rules.mk
DIR:=$(CUR)/ps2
include $(DIR)/rules.mk
DIR:=$(CUR)/ps2keyboard
include $(DIR)/rules.mk
DIR:=$(CUR)/ps2mouse
include $(DIR)/rules.mk
DIR:=$(CUR)/rtc
include $(DIR)/rules.mk
DIR:=$(CUR)/rtl8139
include $(DIR)/rules.mk
DIR:=$(CUR)/smp
include $(DIR)/rules.mk
DIR:=$(CUR)/uart
include $(DIR)/rules.mk
DIR:=$(CUR)/uhci
include $(DIR)/rules.mk
DIR:=$(CUR)/vga
include $(DIR)/rules.mk
DIR:=$(CUR)/virtio
include $(DIR)/rules.mk

CUR:=$(TMP_$(PAR))
PAR:=$(basename $(PAR))
