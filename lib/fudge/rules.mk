L:=\
    $(DIR_LIB)/fudge/fudge.a \

O:=\
    $(DIR_LIB)/fudge/ascii.o \
    $(DIR_LIB)/fudge/atomic.o \
    $(DIR_LIB)/fudge/ctrl.o \
    $(DIR_LIB)/fudge/list.o \
    $(DIR_LIB)/fudge/memory.o \
    $(DIR_LIB)/fudge/ring.o \
    $(DIR_LIB)/fudge/spinlock.o \

include $(DIR_MK)/lib.mk
