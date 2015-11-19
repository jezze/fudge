BIN_MANDELBROT:=\
    $(SRC_PATH)/mandelbrot/mandelbrot \

OBJ_MANDELBROT:=\
    $(SRC_PATH)/mandelbrot/mandelbrot.o \
    $(SRC_PATH)/fudge/ctrl.o \
    $(SRC_PATH)/lib/file.o \
    $(SRC_PATH)/lib/video.o \

$(BIN_MANDELBROT): $(OBJ_MANDELBROT) $(OBJ_STD) $(OBJ_ABI)
	$(LD) -o $@ $(LDFLAGS) $^

BIN:=$(BIN) $(BIN_MANDELBROT)
CLEAN:=$(CLEAN) $(BIN_MANDELBROT) $(OBJ_MANDELBROT)
