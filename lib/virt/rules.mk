L:=\
    $(DIR_LIB)/virt/virt.a \

O:=\
    $(DIR_LIB)/virt/cpio.o \
    $(DIR_LIB)/virt/init.o \
    $(DIR_LIB)/virt/virt.o \

include $(DIR_LIB)/virt/$(ARCH)/rules.mk
include $(DIR_MK)/lib.mk
