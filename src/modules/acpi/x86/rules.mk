M:=\
    $(DIR_SRC)/modules/acpi/x86/acpi.ko \

N:=\
    $(DIR_SRC)/modules/acpi/x86/acpi.ko.map \

O:=\
    $(DIR_SRC)/modules/acpi/x86/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
