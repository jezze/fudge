BIN:=packages/mandelbrot/mandelbrot
OBJ:=packages/mandelbrot/mandelbrot.o

$(BIN): $(OBJ) $(USERLIBS)
	$(LD) $(LDFLAGS) -o $@ $^

PACKAGES+=$(BIN)
PACKAGES_OBJECTS+=$(OBJ)
