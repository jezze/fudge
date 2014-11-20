PAR:=$(PAR).x
TMP_$(PAR):=$(CUR)
CUR:=$(DIR)

DIR:=$(CUR)/$(ARCH)
include $(DIR)/rules.mk

CUR:=$(TMP_$(PAR))
PAR:=$(basename $(PAR))
