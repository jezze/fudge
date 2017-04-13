B:=\
    $(DIR_SRC)/mandelbrot/mandelbrot \

O:=\
    $(DIR_SRC)/mandelbrot/mandelbrot.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk
