L:=\
    $(DIR_LIB)/hash/hash.a \

O:=\
    $(DIR_LIB)/hash/crc.o \
    $(DIR_LIB)/hash/md5.o \
    $(DIR_LIB)/hash/sha1.o \

include $(DIR_MK)/lib.mk
