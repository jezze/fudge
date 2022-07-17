B:=\
    $(DIR_SRC)/wm2/wm2 \

O:=\
    $(DIR_SRC)/wm2/wm2.o \
    $(DIR_SRC)/wm2/widget.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk
