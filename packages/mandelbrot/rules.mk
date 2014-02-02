BIN:=$(PACKAGES_PATH)/mandelbrot/mandelbrot
OBJ:=$(PACKAGES_PATH)/mandelbrot/mandelbrot.o

$(BIN): $(OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN)
PACKAGES_OBJECTS+=$(OBJ)
