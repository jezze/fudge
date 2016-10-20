L:=\
    $(DIR_SRC)/format/format.a \

O:=\
    $(DIR_SRC)/format/cpio.o \
    $(DIR_SRC)/format/elf.o \
    $(DIR_SRC)/format/md5.o \
    $(DIR_SRC)/format/pcf.o \
    $(DIR_SRC)/format/sha1.o \
    $(DIR_SRC)/format/tar.o \
    $(DIR_SRC)/format/utf8.o \

include $(DIR_MK)/lib.mk
