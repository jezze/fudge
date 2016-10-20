B:=\
    $(DIR_SRC)/wm/wdraw \

O:=\
    $(DIR_SRC)/wm/wdraw.o \
    $(DIR_SRC)/wm/box.o \
    $(DIR_SRC)/wm/element.o \

L:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \
    $(DIR_SRC)/format/format.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/wm/wedit \

O:=\
    $(DIR_SRC)/wm/wedit.o \
    $(DIR_SRC)/wm/box.o \
    $(DIR_SRC)/wm/element.o \
    $(DIR_SRC)/wm/print.o \
    $(DIR_SRC)/wm/send.o \

L:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/wm/wm \

O:=\
    $(DIR_SRC)/wm/wm.o \
    $(DIR_SRC)/wm/box.o \
    $(DIR_SRC)/wm/element.o \
    $(DIR_SRC)/wm/print.o \
    $(DIR_SRC)/wm/send.o \

L:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/wm/wshell \

O:=\
    $(DIR_SRC)/wm/wshell.o \
    $(DIR_SRC)/wm/box.o \
    $(DIR_SRC)/wm/element.o \
    $(DIR_SRC)/wm/send.o \
    $(DIR_SRC)/wm/print.o \
    $(DIR_SRC)/wm/keymap.o \

L:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/wm/wtext \

O:=\
    $(DIR_SRC)/wm/wtext.o \
    $(DIR_SRC)/wm/box.o \
    $(DIR_SRC)/wm/element.o \

L:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/bin.mk
