B:=\
    $(DIR_SRC)/sha/sha1 \

O:=\
    $(DIR_SRC)/sha/sha1.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/sha/sha.a \

include $(DIR_MK)/bin.mk
