M:=\
    $(DIR_SRC)/modules/arch/x86/virtio/virtio.ko.0 \

O:=\
    $(DIR_SRC)/modules/arch/x86/virtio/main.o \

L:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
