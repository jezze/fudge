BIN_UTILS:=\
    $(DIR_SRC)/utils/cat \
    $(DIR_SRC)/utils/cconv8to6 \
    $(DIR_SRC)/utils/complete \
    $(DIR_SRC)/utils/countb \
    $(DIR_SRC)/utils/countl \
    $(DIR_SRC)/utils/crc \
    $(DIR_SRC)/utils/ctrl \
    $(DIR_SRC)/utils/date \
    $(DIR_SRC)/utils/dump \
    $(DIR_SRC)/utils/echo \
    $(DIR_SRC)/utils/hello \
    $(DIR_SRC)/utils/help \
    $(DIR_SRC)/utils/initfs \
    $(DIR_SRC)/utils/init \
    $(DIR_SRC)/utils/initmod \
    $(DIR_SRC)/utils/ipsetup \
    $(DIR_SRC)/utils/keymapse \
    $(DIR_SRC)/utils/keymapus \
    $(DIR_SRC)/utils/lsc \
    $(DIR_SRC)/utils/ls \
    $(DIR_SRC)/utils/mbr \
    $(DIR_SRC)/utils/md5 \
    $(DIR_SRC)/utils/motd \
    $(DIR_SRC)/utils/named \
    $(DIR_SRC)/utils/namef \
    $(DIR_SRC)/utils/reboot \
    $(DIR_SRC)/utils/sha1 \
    $(DIR_SRC)/utils/shell \
    $(DIR_SRC)/utils/slang \

OBJ_UTILS:=\
    $(DIR_SRC)/utils/cat.o \
    $(DIR_SRC)/utils/cconv8to6.o \
    $(DIR_SRC)/utils/complete.o \
    $(DIR_SRC)/utils/countb.o \
    $(DIR_SRC)/utils/countl.o \
    $(DIR_SRC)/utils/crc.o \
    $(DIR_SRC)/utils/ctrl.o \
    $(DIR_SRC)/utils/date.o \
    $(DIR_SRC)/utils/dump.o \
    $(DIR_SRC)/utils/echo.o \
    $(DIR_SRC)/utils/hello.o \
    $(DIR_SRC)/utils/help.o \
    $(DIR_SRC)/utils/initfs.o \
    $(DIR_SRC)/utils/init.o \
    $(DIR_SRC)/utils/initmod.o \
    $(DIR_SRC)/utils/ipsetup.o \
    $(DIR_SRC)/utils/keymapse.o \
    $(DIR_SRC)/utils/keymapus.o \
    $(DIR_SRC)/utils/lsc.o \
    $(DIR_SRC)/utils/ls.o \
    $(DIR_SRC)/utils/mbr.o \
    $(DIR_SRC)/utils/md5.o \
    $(DIR_SRC)/utils/motd.o \
    $(DIR_SRC)/utils/named.o \
    $(DIR_SRC)/utils/namef.o \
    $(DIR_SRC)/utils/reboot.o \
    $(DIR_SRC)/utils/sha1.o \
    $(DIR_SRC)/utils/shell.o \
    $(DIR_SRC)/utils/slang.o \

OBJ_EXTRA:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \
    $(DIR_SRC)/format/format.a \

$(BIN_UTILS): % : %.o $(OBJ_EXTRA)
	@echo LD $@: $^
	@$(LD) $(LDFLAGS) -o $@ $^

BIN:=$(BIN) $(BIN_UTILS)
OBJ:=$(OBJ) $(OBJ_UTILS) $(OBJ_EXTRA)
