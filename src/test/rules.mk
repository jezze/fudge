B:=\
    $(DIR_SRC)/test/t_disk \

O:=\
    $(DIR_SRC)/test/t_disk.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/test/t_input \

O:=\
    $(DIR_SRC)/test/t_input.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/test/t_timer \

O:=\
    $(DIR_SRC)/test/t_timer.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk
