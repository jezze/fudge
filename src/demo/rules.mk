B:=\
    $(DIR_SRC)/demo/demo \

O:=\
    $(DIR_SRC)/demo/demo.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

$(DIR_SRC)/demo/demo.o: CC=$(TARGET)-cc -Wall -Werror -Wno-overlength-strings -ffreestanding -fno-asynchronous-unwind-tables -std=c89 -O2 -S -o

include $(DIR_MK)/bin.mk
