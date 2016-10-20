B:=\
    $(DIR_SRC)/mandelbrot/mandelbrot \

O:=\
    $(DIR_SRC)/mandelbrot/mandelbrot.o \

L:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \

include $(DIR_MK)/bin.mk
