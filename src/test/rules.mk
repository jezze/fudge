B:=\
    $(DIR_SRC)/test/t_disk \

O:=\
    $(DIR_SRC)/test/t_disk.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/socket/socket.a \
    $(DIR_LIB)/net/net.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/test/t_disk2 \

O:=\
    $(DIR_SRC)/test/t_disk2.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/test/t_donut \

O:=\
    $(DIR_SRC)/test/t_donut.o \

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
    $(DIR_SRC)/test/t_loop \

O:=\
    $(DIR_SRC)/test/t_loop.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/test/t_mandelbrot \

O:=\
    $(DIR_SRC)/test/t_mandelbrot.o \

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
