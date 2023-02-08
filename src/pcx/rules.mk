B:=\
    $(DIR_SRC)/pcx/pcxcmap \

O:=\
    $(DIR_SRC)/pcx/pcxcmap.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/image/image.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/pcx/pcxdata \

O:=\
    $(DIR_SRC)/pcx/pcxdata.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/image/image.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/pcx/pcxinfo \

O:=\
    $(DIR_SRC)/pcx/pcxinfo.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/image/image.a \

include $(DIR_MK)/bin.mk
