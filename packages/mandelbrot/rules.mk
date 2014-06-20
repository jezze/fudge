BIN_$(DIR):=$(DIR)/mandelbrot
OBJ_$(DIR):=$(DIR)/mandelbrot.o

$(BIN_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $^ $(LDFLAGS)

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
