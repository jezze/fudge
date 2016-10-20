M:=\
    $(DIR_SRC)/modules/arch/x86/rtc/rtc.ko.0 \

O:=\
    $(DIR_SRC)/modules/arch/x86/rtc/main.o \

L:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
