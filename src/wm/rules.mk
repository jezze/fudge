B:=\
    $(DIR_SRC)/wm/wedit \

O:=\
    $(DIR_SRC)/wm/wedit.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/event/event.a \
    $(DIR_LIB)/keymap/keymap.a \
    $(DIR_LIB)/math/math.a \
    $(DIR_LIB)/widget/widget.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/wm/wres \

O:=\
    $(DIR_SRC)/wm/wres.o \
    $(DIR_SRC)/wm/render.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/event/event.a \
    $(DIR_LIB)/math/math.a \
    $(DIR_LIB)/pcf/pcf.a \
    $(DIR_LIB)/widget/widget.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/wm/wm \

O:=\
    $(DIR_SRC)/wm/wm.o \
    $(DIR_SRC)/wm/render.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/event/event.a \
    $(DIR_LIB)/keymap/keymap.a \
    $(DIR_LIB)/math/math.a \
    $(DIR_LIB)/pcf/pcf.a \
    $(DIR_LIB)/widget/widget.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/wm/wshell \

O:=\
    $(DIR_SRC)/wm/wshell.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/event/event.a \
    $(DIR_LIB)/keymap/keymap.a \
    $(DIR_LIB)/math/math.a \
    $(DIR_LIB)/widget/widget.a \

include $(DIR_MK)/bin.mk
