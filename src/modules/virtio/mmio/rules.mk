M:=\
    $(DIR_SRC)/modules/virtio/virtio-network.ko \

N:=\
    $(DIR_SRC)/modules/virtio/virtio-network.ko.map \

O:=\
    $(DIR_SRC)/modules/virtio/mmio/virtio.o \
    $(DIR_SRC)/modules/virtio/queue.o \
    $(DIR_SRC)/modules/virtio/network.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/virtio/virtio-block.ko \

N:=\
    $(DIR_SRC)/modules/virtio/virtio-block.ko.map \

O:=\
    $(DIR_SRC)/modules/virtio/mmio/virtio.o \
    $(DIR_SRC)/modules/virtio/queue.o \
    $(DIR_SRC)/modules/virtio/block.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/virtio/virtio-keyboard.ko \

N:=\
    $(DIR_SRC)/modules/virtio/virtio-keyboard.ko.map \

O:=\
    $(DIR_SRC)/modules/virtio/mmio/virtio.o \
    $(DIR_SRC)/modules/virtio/queue.o \
    $(DIR_SRC)/modules/virtio/input.o \
    $(DIR_SRC)/modules/virtio/keyboard.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/virtio/virtio-mouse.ko \

N:=\
    $(DIR_SRC)/modules/virtio/virtio-mouse.ko.map \

O:=\
    $(DIR_SRC)/modules/virtio/mmio/virtio.o \
    $(DIR_SRC)/modules/virtio/queue.o \
    $(DIR_SRC)/modules/virtio/input.o \
    $(DIR_SRC)/modules/virtio/mouse.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/virtio/virtio-gpu.ko \

N:=\
    $(DIR_SRC)/modules/virtio/virtio-gpu.ko.map \

O:=\
    $(DIR_SRC)/modules/virtio/mmio/virtio.o \
    $(DIR_SRC)/modules/virtio/queue.o \
    $(DIR_SRC)/modules/virtio/gpu.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
