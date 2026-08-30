L:=\
    $(DIR_LIB)/disk/disk.a \

O:=\
    $(DIR_LIB)/disk/cpio.o \
    $(DIR_LIB)/disk/mbr.o \
    $(DIR_LIB)/disk/tar.o \

include $(DIR_MK)/lib.mk
