M:=\
    $(DIR_SRC)/modules/arch/x86/i915/i915.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/i915/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
