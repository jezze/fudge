PAR:=$(PAR).x
TMP_$(PAR):=$(CUR)
CUR:=$(DIR)

include $(CUR)/rules.$(ARCH).mk

DIR:=$(CUR)/abi
include $(DIR)/rules.mk
DIR:=$(CUR)/arch
include $(DIR)/rules.mk
DIR:=$(CUR)/fudge
include $(DIR)/rules.mk
DIR:=$(CUR)/kernel
include $(DIR)/rules.mk
DIR:=$(CUR)/cpio
include $(DIR)/rules.mk
DIR:=$(CUR)/elf
include $(DIR)/rules.mk
DIR:=$(CUR)/tar
include $(DIR)/rules.mk
DIR:=$(CUR)/utf8
include $(DIR)/rules.mk

CUR:=$(TMP_$(PAR))
PAR:=$(basename $(PAR))
