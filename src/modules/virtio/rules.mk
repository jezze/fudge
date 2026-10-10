M:=\
    $(DIR_SRC)/modules/virtio/virtio-network.ko \

N:=\
    $(DIR_SRC)/modules/virtio/virtio-network.ko.map \

O:=\
    $(DIR_SRC)/modules/virtio/virtio.o \
    $(DIR_SRC)/modules/virtio/network.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/virtio/virtio-block.ko \

N:=\
    $(DIR_SRC)/modules/virtio/virtio-block.ko.map \

O:=\
    $(DIR_SRC)/modules/virtio/virtio.o \
    $(DIR_SRC)/modules/virtio/block.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/net/net.a \

include $(DIR_MK)/kmod.mk
