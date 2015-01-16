PAR:=$(PAR).x
TMP_$(PAR):=$(CUR)
CUR:=$(DIR)

DIR:=$(CUR)/cat
include $(DIR)/rules.mk
DIR:=$(CUR)/cconv
include $(DIR)/rules.mk
DIR:=$(CUR)/col
include $(DIR)/rules.mk
DIR:=$(CUR)/complete
include $(DIR)/rules.mk
DIR:=$(CUR)/count
include $(DIR)/rules.mk
DIR:=$(CUR)/cpio
include $(DIR)/rules.mk
DIR:=$(CUR)/crc
include $(DIR)/rules.mk
DIR:=$(CUR)/ctrl
include $(DIR)/rules.mk
DIR:=$(CUR)/date
include $(DIR)/rules.mk
DIR:=$(CUR)/dep
include $(DIR)/rules.mk
DIR:=$(CUR)/dump
include $(DIR)/rules.mk
DIR:=$(CUR)/echo
include $(DIR)/rules.mk
DIR:=$(CUR)/gfx
include $(DIR)/rules.mk
DIR:=$(CUR)/head
include $(DIR)/rules.mk
DIR:=$(CUR)/hello
include $(DIR)/rules.mk
DIR:=$(CUR)/help
include $(DIR)/rules.mk
DIR:=$(CUR)/init
include $(DIR)/rules.mk
DIR:=$(CUR)/keymap
include $(DIR)/rules.mk
DIR:=$(CUR)/ls
include $(DIR)/rules.mk
DIR:=$(CUR)/mbr
include $(DIR)/rules.mk
DIR:=$(CUR)/md5
include $(DIR)/rules.mk
DIR:=$(CUR)/mod
include $(DIR)/rules.mk
DIR:=$(CUR)/motd
include $(DIR)/rules.mk
DIR:=$(CUR)/mount
include $(DIR)/rules.mk
DIR:=$(CUR)/name
include $(DIR)/rules.mk
DIR:=$(CUR)/nl
include $(DIR)/rules.mk
DIR:=$(CUR)/pcx
include $(DIR)/rules.mk
DIR:=$(CUR)/reboot
include $(DIR)/rules.mk
DIR:=$(CUR)/sha1
include $(DIR)/rules.mk
DIR:=$(CUR)/share
include $(DIR)/rules.mk
DIR:=$(CUR)/shell
include $(DIR)/rules.mk
DIR:=$(CUR)/slang
include $(DIR)/rules.mk
DIR:=$(CUR)/sleep
include $(DIR)/rules.mk
DIR:=$(CUR)/wm
include $(DIR)/rules.mk

CUR:=$(TMP_$(PAR))
PAR:=$(basename $(PAR))
