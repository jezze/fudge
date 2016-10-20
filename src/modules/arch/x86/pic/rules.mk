M:=\
    $(DIR_SRC)/modules/arch/x86/pic/pic.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/pic/main.o \
    $(DIR_SRC)/modules/arch/x86/pic/pic.o \

D:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
