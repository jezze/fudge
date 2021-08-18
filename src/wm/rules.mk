B:=\
    $(DIR_SRC)/wm/wedit \

O:=\
    $(DIR_SRC)/wm/wedit.o \

L:=\
    $(DIR_LIB)/widget/widget.a \
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/wm/wres \

O:=\
    $(DIR_SRC)/wm/wres.o \

L:=\
    $(DIR_LIB)/widget/widget.a \
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/wm/wm \

O:=\
    $(DIR_SRC)/wm/wm.o \

L:=\
    $(DIR_LIB)/widget/widget.a \
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/wm/wm2 \

O:=\
    $(DIR_SRC)/wm/wm2.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/wm/wshell \

O:=\
    $(DIR_SRC)/wm/wshell.o \

L:=\
    $(DIR_LIB)/widget/widget.a \
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk
