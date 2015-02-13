PAR:=$(PAR).x
TMP_$(PAR):=$(CUR)
CUR:=$(DIR)

DIR:=$(CUR)/ethernet
include $(DIR)/rules.mk
DIR:=$(CUR)/arp
include $(DIR)/rules.mk
DIR:=$(CUR)/ipv4
include $(DIR)/rules.mk
DIR:=$(CUR)/ipv6
include $(DIR)/rules.mk
DIR:=$(CUR)/udp
include $(DIR)/rules.mk

CUR:=$(TMP_$(PAR))
PAR:=$(basename $(PAR))
