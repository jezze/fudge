B:=\
    $(DIR_SRC)/md5/md5 \

O:=\
    $(DIR_SRC)/md5/md5.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/md5/md5.a \

include $(DIR_MK)/bin.mk
