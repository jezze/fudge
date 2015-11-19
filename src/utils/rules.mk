BIN_UTILS:=\
    $(SRC_PATH)/utils/cat \
    $(SRC_PATH)/utils/cconv8to6 \
    $(SRC_PATH)/utils/col \
    $(SRC_PATH)/utils/complete \
    $(SRC_PATH)/utils/countb \
    $(SRC_PATH)/utils/countl \
    $(SRC_PATH)/utils/crc \
    $(SRC_PATH)/utils/ctrl \
    $(SRC_PATH)/utils/date \
    $(SRC_PATH)/utils/dump \
    $(SRC_PATH)/utils/echo \
    $(SRC_PATH)/utils/headb \
    $(SRC_PATH)/utils/headl \
    $(SRC_PATH)/utils/hello \
    $(SRC_PATH)/utils/help \
    $(SRC_PATH)/utils/initfs \
    $(SRC_PATH)/utils/init \
    $(SRC_PATH)/utils/initmod \
    $(SRC_PATH)/utils/keymapse \
    $(SRC_PATH)/utils/keymapus \
    $(SRC_PATH)/utils/lsc \
    $(SRC_PATH)/utils/ls \
    $(SRC_PATH)/utils/mbr \
    $(SRC_PATH)/utils/md5 \
    $(SRC_PATH)/utils/motd \
    $(SRC_PATH)/utils/mount \
    $(SRC_PATH)/utils/named \
    $(SRC_PATH)/utils/namef \
    $(SRC_PATH)/utils/nl \
    $(SRC_PATH)/utils/reboot \
    $(SRC_PATH)/utils/sha1 \
    $(SRC_PATH)/utils/shell \
    $(SRC_PATH)/utils/slang \
    $(SRC_PATH)/utils/sleep \

OBJ_UTILS:=\
    $(SRC_PATH)/utils/cat.o \
    $(SRC_PATH)/utils/cconv8to6.o \
    $(SRC_PATH)/utils/col.o \
    $(SRC_PATH)/utils/complete.o \
    $(SRC_PATH)/utils/countb.o \
    $(SRC_PATH)/utils/countl.o \
    $(SRC_PATH)/utils/crc.o \
    $(SRC_PATH)/utils/ctrl.o \
    $(SRC_PATH)/utils/date.o \
    $(SRC_PATH)/utils/dump.o \
    $(SRC_PATH)/utils/echo.o \
    $(SRC_PATH)/utils/headb.o \
    $(SRC_PATH)/utils/headl.o \
    $(SRC_PATH)/utils/hello.o \
    $(SRC_PATH)/utils/help.o \
    $(SRC_PATH)/utils/initfs.o \
    $(SRC_PATH)/utils/init.o \
    $(SRC_PATH)/utils/initmod.o \
    $(SRC_PATH)/utils/keymapse.o \
    $(SRC_PATH)/utils/keymapus.o \
    $(SRC_PATH)/utils/lsc.o \
    $(SRC_PATH)/utils/ls.o \
    $(SRC_PATH)/utils/mbr.o \
    $(SRC_PATH)/utils/md5.o \
    $(SRC_PATH)/utils/motd.o \
    $(SRC_PATH)/utils/mount.o \
    $(SRC_PATH)/utils/named.o \
    $(SRC_PATH)/utils/namef.o \
    $(SRC_PATH)/utils/nl.o \
    $(SRC_PATH)/utils/reboot.o \
    $(SRC_PATH)/utils/sha1.o \
    $(SRC_PATH)/utils/shell.o \
    $(SRC_PATH)/utils/slang.o \
    $(SRC_PATH)/utils/sleep.o \

OBJ_EXTRA:=\
    $(SRC_PATH)/fudge/ascii.o \
    $(SRC_PATH)/fudge/buffer.o \
    $(SRC_PATH)/fudge/ctrl.o \
    $(SRC_PATH)/fudge/list.o \
    $(SRC_PATH)/fudge/memory.o \
    $(SRC_PATH)/lib/file.o \

$(BIN_UTILS): % : %.o $(OBJ_EXTRA) $(OBJ_STD) $(OBJ_ABI)
	$(LD) -o $@ $(LDFLAGS) $^

BIN:=$(BIN) $(BIN_UTILS)
CLEAN:=$(CLEAN) $(BIN_UTILS) $(OBJ_UTILS)
