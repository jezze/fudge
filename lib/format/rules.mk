L:=\
    $(DIR_LIB)/format/format.a \

O:=\
    $(DIR_LIB)/format/cpio.o \
    $(DIR_LIB)/format/elf.o \
    $(DIR_LIB)/format/pcf.o \
    $(DIR_LIB)/format/tar.o \

include $(DIR_MK)/lib.mk
