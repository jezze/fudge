M:=\
    $(DIR_SRC)/modules/pipe/pipe.ko \

O:=\
    $(DIR_SRC)/modules/pipe/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/pipe/pipe.ko.0 \

O:=\
    $(DIR_SRC)/modules/pipe/node.o \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/pipe/pipe.ko.1 \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/pipe/pipe.ko.2 \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/pipe/pipe.ko.3 \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/pipe/pipe.ko.4 \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/pipe/pipe.ko.5 \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/pipe/pipe.ko.6 \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/pipe/pipe.ko.7 \

include $(DIR_MK)/kmod.mk
