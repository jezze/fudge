packages/%.o: CFLAGS+=-Ilibs/arch -Ilibs/fudge

USERLIBS:=$(LIBARCH) $(LIBFUDGE)

include packages/cat/rules.mk
include packages/cconv/rules.mk
include packages/col/rules.mk
include packages/complete/rules.mk
include packages/count/rules.mk
include packages/cpio/rules.mk
include packages/crc/rules.mk
include packages/date/rules.mk
include packages/dump/rules.mk
include packages/echo/rules.mk
include packages/hello/rules.mk
include packages/help/rules.mk
include packages/init/rules.mk
include packages/keymap/rules.mk
include packages/ls/rules.mk
include packages/md5/rules.mk
include packages/mod/rules.mk
include packages/motd/rules.mk
include packages/mount/rules.mk
include packages/name/rules.mk
include packages/nl/rules.mk
include packages/pcx/rules.mk
include packages/reboot/rules.mk
include packages/sha1/rules.mk
include packages/shell/rules.mk
include packages/slang/rules.mk
include packages/tail/rules.mk
include packages/timer/rules.mk
include packages/wait/rules.mk
include packages/wm/rules.mk
