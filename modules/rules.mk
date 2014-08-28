PAR:=$(PAR).x
TMP_$(PAR):=$(CUR)
CUR:=$(DIR)

DIR:=$(CUR)/base
include $(DIR)/rules.mk
DIR:=$(CUR)/block
include $(DIR)/rules.mk
DIR:=$(CUR)/log
include $(DIR)/rules.mk
DIR:=$(CUR)/mtwist
include $(DIR)/rules.mk
DIR:=$(CUR)/network
include $(DIR)/rules.mk
DIR:=$(CUR)/null
include $(DIR)/rules.mk
DIR:=$(CUR)/pipe
include $(DIR)/rules.mk
DIR:=$(CUR)/system
include $(DIR)/rules.mk
DIR:=$(CUR)/arch/$(ARCH)
include $(DIR)/rules.mk

CUR:=$(TMP_$(PAR))
PAR:=$(basename $(PAR))
