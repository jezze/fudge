L:=\
    $(DIR_LIB)/mboot/mboot.a \

O:=\
    $(DIR_LIB)/mboot/cpio.o \
    $(DIR_LIB)/mboot/elf.o \
    $(DIR_LIB)/mboot/init.o \
    $(DIR_LIB)/mboot/mboot.o \

LD_SCRIPT_KBIN:=$(DIR_LIB)/mboot/linker.ld

include $(DIR_MK)/lib.mk
