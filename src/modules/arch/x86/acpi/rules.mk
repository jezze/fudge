M:=\
    $(DIR_SRC)/modules/arch/x86/acpi/acpi.ko \

O:=\
    $(DIR_SRC)/modules/arch/x86/acpi/main.o \

L:=\
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/mod.mk
