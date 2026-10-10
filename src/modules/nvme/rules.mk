M:=\
    $(DIR_SRC)/modules/nvme/nvme.ko \

N:=\
    $(DIR_SRC)/modules/nvme/nvme.ko.map \

O:=\
    $(DIR_SRC)/modules/nvme/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
