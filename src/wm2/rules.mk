B:=\
    $(DIR_SRC)/wm2/wm2 \

O:=\
    $(DIR_SRC)/wm2/parser.o \
    $(DIR_SRC)/wm2/place.o \
    $(DIR_SRC)/wm2/pool.o \
    $(DIR_SRC)/wm2/render.o \
    $(DIR_SRC)/wm2/util.o \
    $(DIR_SRC)/wm2/widget.o \
    $(DIR_SRC)/wm2/wm2.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/wm2/wcalc \

O:=\
    $(DIR_SRC)/wm2/wcalc.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/wm2/wshell2 \

O:=\
    $(DIR_SRC)/wm2/wshell2.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/wm2/wtest \

O:=\
    $(DIR_SRC)/wm2/wtest.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk
