BIN_MANDELBROT:=\
    $(DIR_SRC)/mandelbrot/mandelbrot \

OBJ_MANDELBROT:=\
    $(DIR_SRC)/mandelbrot/mandelbrot.o \
    $(DIR_SRC)/fudge/ctrl.o \
    $(DIR_SRC)/lib/file.o \
    $(DIR_SRC)/lib/video.o \

$(BIN_MANDELBROT): $(OBJ_MANDELBROT) $(OBJ_STD) $(OBJ_ABI)
	$(LD) -o $@ $(LDFLAGS) $^

BIN:=$(BIN) $(BIN_MANDELBROT)
CLEAN:=$(CLEAN) $(BIN_MANDELBROT) $(OBJ_MANDELBROT)
