B:=\
    $(DIR_SRC)/utils/dump \

O:=\
    $(DIR_SRC)/utils/dump.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/common/common.a \
    $(DIR_LIB)/event/event.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/echo \

O:=\
    $(DIR_SRC)/utils/echo.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/common/common.a \
    $(DIR_LIB)/event/event.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/elfload \

O:=\
    $(DIR_SRC)/utils/elfload.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/common/common.a \
    $(DIR_LIB)/format/format.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/elfunload \

O:=\
    $(DIR_SRC)/utils/elfunload.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/common/common.a \
    $(DIR_LIB)/format/format.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/help \

O:=\
    $(DIR_SRC)/utils/help.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/common/common.a \
    $(DIR_LIB)/event/event.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/init \

O:=\
    $(DIR_SRC)/utils/init.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/common/common.a \
    $(DIR_LIB)/event/event.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/initfs \

O:=\
    $(DIR_SRC)/utils/initfs.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/common/common.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/initnet \

O:=\
    $(DIR_SRC)/utils/initnet.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/common/common.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/ls \

O:=\
    $(DIR_SRC)/utils/ls.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/common/common.a \
    $(DIR_LIB)/event/event.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/lsc \

O:=\
    $(DIR_SRC)/utils/lsc.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/common/common.a \
    $(DIR_LIB)/event/event.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/motd \

O:=\
    $(DIR_SRC)/utils/motd.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/common/common.a \
    $(DIR_LIB)/event/event.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/reboot \

O:=\
    $(DIR_SRC)/utils/reboot.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/common/common.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/shell \

O:=\
    $(DIR_SRC)/utils/shell.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/common/common.a \
    $(DIR_LIB)/event/event.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/utils/slang \

O:=\
    $(DIR_SRC)/utils/slang.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/common/common.a \
    $(DIR_LIB)/event/event.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk
