B:=\
    $(DIR_SRC)/utils/cat \

O:=\
    $(DIR_SRC)/utils/cat.o \

L:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/cconv8to6 \

O:=\
    $(DIR_SRC)/utils/cconv8to6.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/complete \

O:=\
    $(DIR_SRC)/utils/complete.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/countb \

O:=\
    $(DIR_SRC)/utils/countb.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/countl \

O:=\
    $(DIR_SRC)/utils/countl.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/crc \

O:=\
    $(DIR_SRC)/utils/crc.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/ctrl \

O:=\
    $(DIR_SRC)/utils/ctrl.o \
    $(DIR_SRC)/utils/write.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/date \

O:=\
    $(DIR_SRC)/utils/date.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/disksetup \

O:=\
    $(DIR_SRC)/utils/disksetup.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/dump \

O:=\
    $(DIR_SRC)/utils/dump.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/echo \

O:=\
    $(DIR_SRC)/utils/echo.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/fat \

O:=\
    $(DIR_SRC)/utils/fat.o \
    $(DIR_SRC)/utils/write.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/hello \

O:=\
    $(DIR_SRC)/utils/hello.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/help \

O:=\
    $(DIR_SRC)/utils/help.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/initfs \

O:=\
    $(DIR_SRC)/utils/initfs.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/init \

O:=\
    $(DIR_SRC)/utils/init.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/initmod \

O:=\
    $(DIR_SRC)/utils/initmod.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/ipsetup \

O:=\
    $(DIR_SRC)/utils/ipsetup.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/ipserver \

O:=\
    $(DIR_SRC)/utils/ipserver.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/keymapse \

O:=\
    $(DIR_SRC)/utils/keymapse.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/keymapus \

O:=\
    $(DIR_SRC)/utils/keymapus.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/lsc \

O:=\
    $(DIR_SRC)/utils/lsc.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/ls \

O:=\
    $(DIR_SRC)/utils/ls.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/mbr \

O:=\
    $(DIR_SRC)/utils/mbr.o \
    $(DIR_SRC)/utils/write.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/motd \

O:=\
    $(DIR_SRC)/utils/motd.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/named \

O:=\
    $(DIR_SRC)/utils/named.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/namef \

O:=\
    $(DIR_SRC)/utils/namef.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/reboot \

O:=\
    $(DIR_SRC)/utils/reboot.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/shell \

O:=\
    $(DIR_SRC)/utils/shell.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/slang \

O:=\
    $(DIR_SRC)/utils/slang.o \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/timestamp \

O:=\
    $(DIR_SRC)/utils/timestamp.o \

include $(DIR_MK)/bin.mk
