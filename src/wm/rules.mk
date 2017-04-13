B:=\
    $(DIR_SRC)/wm/wedit \

O:=\
    $(DIR_SRC)/wm/wedit.o \
    $(DIR_SRC)/wm/box.o \
    $(DIR_SRC)/wm/element.o \
    $(DIR_SRC)/wm/ev.o \
    $(DIR_SRC)/wm/keymap.o \
    $(DIR_SRC)/wm/print.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/wm/wres \

O:=\
    $(DIR_SRC)/wm/wres.o \
    $(DIR_SRC)/wm/box.o \
    $(DIR_SRC)/wm/element.o \
    $(DIR_SRC)/wm/ev.o \
    $(DIR_SRC)/wm/print.o \
    $(DIR_SRC)/wm/render.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/pcf/pcf.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/wm/wm \

O:=\
    $(DIR_SRC)/wm/wm.o \
    $(DIR_SRC)/wm/box.o \
    $(DIR_SRC)/wm/element.o \
    $(DIR_SRC)/wm/ev.o \
    $(DIR_SRC)/wm/print.o \
    $(DIR_SRC)/wm/render.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/pcf/pcf.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/wm/wshell \

O:=\
    $(DIR_SRC)/wm/wshell.o \
    $(DIR_SRC)/wm/box.o \
    $(DIR_SRC)/wm/element.o \
    $(DIR_SRC)/wm/ev.o \
    $(DIR_SRC)/wm/keymap.o \
    $(DIR_SRC)/wm/print.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk
