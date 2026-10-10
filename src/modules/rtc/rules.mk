M:=\
    $(DIR_SRC)/modules/rtc/rtc.ko \

N:=\
    $(DIR_SRC)/modules/rtc/rtc.ko.map \

O:=\
    $(DIR_SRC)/modules/rtc/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
