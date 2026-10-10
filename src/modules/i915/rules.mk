M:=\
    $(DIR_SRC)/modules/i915/i915.ko \

N:=\
    $(DIR_SRC)/modules/i915/i915.ko.map \

O:=\
    $(DIR_SRC)/modules/i915/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
