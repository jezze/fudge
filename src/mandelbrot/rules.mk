BIN_MANDELBROT:=\
    $(SRC_PATH)/mandelbrot/mandelbrot \

OBJ_MANDELBROT:=\
    $(SRC_PATH)/mandelbrot/mandelbrot.o \
    $(SRC_PATH)/lib/video.o \

$(BIN_MANDELBROT): $(OBJ_MANDELBROT) $(SRC_DEPS)
	$(LD) -o $@ $(LDFLAGS) $^

BINS:=$(BINS) $(BIN_MANDELBROT)
CLEAN:=$(CLEAN) $(BIN_MANDELBROT) $(OBJ_MANDELBROT)
