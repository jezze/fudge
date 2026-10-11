M:=\
    $(DIR_SRC)/modules/acpi/acpi.ko \

N:=\
    $(DIR_SRC)/modules/acpi/acpi.ko.map \

O:=\
    $(DIR_SRC)/modules/acpi/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
