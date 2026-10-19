L:=\
    $(DIR_LIB)/mboot2/mboot2.a \

O:=\
    $(DIR_LIB)/mboot2/cpio.o \
    $(DIR_LIB)/mboot2/mboot.o \

include $(DIR_LIB)/mboot2/$(ARCH)/rules.mk
include $(DIR_MK)/lib.mk
