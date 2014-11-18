PAR:=$(PAR).x
TMP_$(PAR):=$(CUR)
CUR:=$(DIR)

DIR:=$(CUR)/fudge
include $(DIR)/rules.mk
DIR:=$(CUR)/kernel
include $(DIR)/rules.mk
DIR:=$(CUR)/posix
include $(DIR)/rules.mk
DIR:=$(CUR)/cpio
include $(DIR)/rules.mk
DIR:=$(CUR)/elf
include $(DIR)/rules.mk
DIR:=$(CUR)/tar
include $(DIR)/rules.mk
DIR:=$(CUR)/utf8
include $(DIR)/rules.mk
DIR:=$(CUR)/$(ARCH)/arch
include $(DIR)/rules.mk
DIR:=$(CUR)/$(ARCH)/abi
include $(DIR)/rules.mk
DIR:=$(CUR)/$(ARCH)/$(LOADER)
include $(DIR)/rules.mk

CUR:=$(TMP_$(PAR))
PAR:=$(basename $(PAR))
