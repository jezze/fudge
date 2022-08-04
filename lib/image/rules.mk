L:=\
    $(DIR_LIB)/image/image.a \

O:=\
    $(DIR_LIB)/image/pcx.o \

include $(DIR_LIB)/fudge/$(ARCH)/rules.mk
include $(DIR_MK)/lib.mk
