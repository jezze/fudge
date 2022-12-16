B:=\
    $(DIR_SRC)/gameboy/gameboy \

O:=\
    $(DIR_SRC)/gameboy/audio.o \
    $(DIR_SRC)/gameboy/cpu.o \
    $(DIR_SRC)/gameboy/gameboy.o \
    $(DIR_SRC)/gameboy/video.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

$(DIR_SRC)/gameboy/audio.o: CC=$(TARGET)-cc -Wall -Werror -Wno-overlength-strings -ffreestanding -fno-asynchronous-unwind-tables -std=c89 -O2 -S -o
$(DIR_SRC)/gameboy/cpu.o: CC=$(TARGET)-cc -Wall -Werror -Wno-overlength-strings -msoft-float -ffreestanding -fno-asynchronous-unwind-tables -std=c89 -O2 -S -o
$(DIR_SRC)/gameboy/gameboy.o: CC=$(TARGET)-cc -Wall -Werror -Wno-overlength-strings -msoft-float -ffreestanding -fno-asynchronous-unwind-tables -std=c89 -O2 -S -o

include $(DIR_MK)/bin.mk
