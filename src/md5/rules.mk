B:=\
    $(DIR_SRC)/md5/md5 \

O:=\
    $(DIR_SRC)/md5/md5.o \

D:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \
    $(DIR_SRC)/format/format.a \

include $(DIR_MK)/bin.mk
