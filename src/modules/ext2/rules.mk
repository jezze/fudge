M:=\
    $(DIR_SRC)/modules/ext2/ext2.ko \

O:=\
    $(DIR_SRC)/modules/ext2/main.o \
    $(DIR_SRC)/modules/ext2/filesystem.o \
    $(DIR_SRC)/modules/ext2/protocol.o \

D:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
