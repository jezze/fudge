M:=\
    $(DIR_SRC)/modules/con/con.ko \

O:=\
    $(DIR_SRC)/modules/con/main.o \

D:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk

M:=\
    $(DIR_SRC)/modules/con/con.ko.0 \

O:=\
    $(DIR_SRC)/modules/con/node.o \

include $(DIR_MK)/mod.mk

M:=\
    $(DIR_SRC)/modules/con/con.ko.1 \

include $(DIR_MK)/mod.mk

M:=\
    $(DIR_SRC)/modules/con/con.ko.2 \

include $(DIR_MK)/mod.mk

M:=\
    $(DIR_SRC)/modules/con/con.ko.3 \

include $(DIR_MK)/mod.mk

M:=\
    $(DIR_SRC)/modules/con/con.ko.4 \

include $(DIR_MK)/mod.mk

M:=\
    $(DIR_SRC)/modules/con/con.ko.5 \

include $(DIR_MK)/mod.mk

M:=\
    $(DIR_SRC)/modules/con/con.ko.6 \

include $(DIR_MK)/mod.mk

M:=\
    $(DIR_SRC)/modules/con/con.ko.7 \

include $(DIR_MK)/mod.mk
