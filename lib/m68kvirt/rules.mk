L:=\
    $(DIR_LIB)/m68kvirt/m68kvirt.a \

O:=\
    $(DIR_LIB)/m68kvirt/cpio.o \
    $(DIR_LIB)/m68kvirt/elf.o \
    $(DIR_LIB)/m68kvirt/init.o \
    $(DIR_LIB)/m68kvirt/m68kvirt.o \

include $(DIR_MK)/lib.mk
