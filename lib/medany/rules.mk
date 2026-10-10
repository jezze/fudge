L:=\
    $(DIR_LIB)/medany/medany.a \

O:=\
    $(DIR_LIB)/medany/init.o \

LD_SCRIPT_KBIN:=$(DIR_LIB)/medany/linker.ld

include $(DIR_MK)/lib.mk
