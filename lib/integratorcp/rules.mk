L:=\
    $(DIR_LIB)/integratorcp/integratorcp.a \

O:=\
    $(DIR_LIB)/integratorcp/init.o \

LD_SCRIPT_KBIN:=$(DIR_LIB)/integratorcp/linker.ld

include $(DIR_MK)/lib.mk
