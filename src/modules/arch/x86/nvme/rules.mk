M:=\
    $(DIR_SRC)/modules/arch/x86/nvme/nvme.ko \

N:=\
    $(DIR_SRC)/modules/arch/x86/nvme/nvme.ko.map \

O:=\
    $(DIR_SRC)/modules/arch/x86/nvme/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
