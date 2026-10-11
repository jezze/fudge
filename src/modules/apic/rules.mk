M:=\
    $(DIR_SRC)/modules/apic/apic.ko \

N:=\
    $(DIR_SRC)/modules/apic/apic.ko.map \

O:=\
    $(DIR_SRC)/modules/apic/main.o \
    $(DIR_SRC)/modules/apic/$(ARCH)/apic.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
