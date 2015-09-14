PAR:=$(PAR).x
TMP_$(PAR):=$(CUR)
CUR:=$(DIR)

DIR:=$(CUR)/kernel
include $(DIR)/rules.mk
DIR:=$(CUR)/mandelbrot
include $(DIR)/mandelbrot.mk
DIR:=$(CUR)/modules
include $(DIR)/rules.mk
DIR:=$(CUR)/pcx
include $(DIR)/pcxcmap.mk
include $(DIR)/pcxdata.mk
DIR:=$(CUR)/utils
include $(DIR)/cat.mk
include $(DIR)/cconv8to6.mk
include $(DIR)/col.mk
include $(DIR)/complete.mk
include $(DIR)/countb.mk
include $(DIR)/countl.mk
include $(DIR)/crc.mk
include $(DIR)/ctrl.mk
include $(DIR)/date.mk
include $(DIR)/dump.mk
include $(DIR)/echo.mk
include $(DIR)/headb.mk
include $(DIR)/headl.mk
include $(DIR)/hello.mk
include $(DIR)/help.mk
include $(DIR)/init.mk
include $(DIR)/initfs.mk
include $(DIR)/initmod.mk
include $(DIR)/keymapse.mk
include $(DIR)/keymapus.mk
include $(DIR)/ls.mk
include $(DIR)/lsc.mk
include $(DIR)/mbr.mk
include $(DIR)/md5.mk
include $(DIR)/modload.mk
include $(DIR)/modunload.mk
include $(DIR)/motd.mk
include $(DIR)/mount.mk
include $(DIR)/named.mk
include $(DIR)/namef.mk
include $(DIR)/nl.mk
include $(DIR)/reboot.mk
include $(DIR)/sha1.mk
include $(DIR)/shell.mk
include $(DIR)/slang.mk
include $(DIR)/sleep.mk
DIR:=$(CUR)/wm
include $(DIR)/wm.mk
include $(DIR)/wnull.mk

CUR:=$(TMP_$(PAR))
PAR:=$(basename $(PAR))
