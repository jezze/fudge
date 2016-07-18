BIN_MANDELBROT:=\
    $(DIR_SRC)/mandelbrot/mandelbrot \

OBJ_MANDELBROT:=\
    $(DIR_SRC)/mandelbrot/mandelbrot.o \
    $(DIR_SRC)/lib/file.o \
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \

$(BIN_MANDELBROT): $(OBJ_MANDELBROT)
	$(LD) -o $@ $(LDFLAGS) $^

BIN:=$(BIN) $(BIN_MANDELBROT)
OBJ:=$(OBJ) $(OBJ_MANDELBROT)
