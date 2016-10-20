M:=\
    $(DIR_SRC)/modules/part/part.ko \

O:=\
    $(DIR_SRC)/modules/part/main.o \

D:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk

M:=\
    $(DIR_SRC)/modules/part/part.ko.0 \

O:=\
    $(DIR_SRC)/modules/part/node.o \

include $(DIR_MK)/mod.mk

M:=\
    $(DIR_SRC)/modules/part/part.ko.1 \

include $(DIR_MK)/mod.mk

M:=\
    $(DIR_SRC)/modules/part/part.ko.2 \

include $(DIR_MK)/mod.mk

M:=\
    $(DIR_SRC)/modules/part/part.ko.3 \

include $(DIR_MK)/mod.mk
