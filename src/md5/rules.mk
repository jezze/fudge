L:=\
    $(DIR_SRC)/md5/libmd5.a \

O:=\
    $(DIR_SRC)/md5/libmd5.o \

include $(DIR_MK)/lib.mk

B:=\
    $(DIR_SRC)/md5/md5 \

O:=\
    $(DIR_SRC)/md5/md5.o \

L:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \
    $(DIR_SRC)/md5/libmd5.a \

include $(DIR_MK)/bin.mk
