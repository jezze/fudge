L:=\
    $(DIR_LIB)/mboot2/mboot2.a \

O:=\
    $(DIR_LIB)/mboot2/cpio.o \

LD_SCRIPT_KBIN:=$(DIR_LIB)/mboot2/linker.ld

include $(DIR_LIB)/mboot2/$(ARCH)/rules.mk
include $(DIR_MK)/lib.mk
