L:=\
    $(DIR_LIB)/virt/virt.a \

O:=\
    $(DIR_LIB)/virt/cpio.o \
    $(DIR_LIB)/virt/elf.o \
    $(DIR_LIB)/virt/init.o \
    $(DIR_LIB)/virt/virt.o \

LD_SCRIPT_KBIN:=$(DIR_LIB)/virt/linker.ld

include $(DIR_MK)/lib.mk
