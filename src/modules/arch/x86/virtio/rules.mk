M:=\
    $(DIR_SRC)/modules/arch/x86/virtio/virtio-network.ko.0 \

O:=\
    $(DIR_SRC)/modules/arch/x86/virtio/network.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/arch/x86/virtio/virtio-block.ko.0 \

O:=\
    $(DIR_SRC)/modules/arch/x86/virtio/block.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
