L:=\
    $(DIR_LIB)/math/math.a \

O:=\
    $(DIR_LIB)/math/math.o \

$(DIR_LIB)/math/math.o: CC=$(TARGET)-cc -Wall -Werror -Wno-overlength-strings -ffreestanding -fno-asynchronous-unwind-tables -std=c89 -O2 -S -o

include $(DIR_MK)/lib.mk
