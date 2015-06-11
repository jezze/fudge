BIN_$(DIR):=$(DIR)/mandelbrot
OBJ_$(DIR):=$(DIR)/mandelbrot.o $(PACKAGES_PATH)/video/video.o

$(BIN_$(DIR)): $(OBJ_$(DIR)) $(PACKAGES_DEPS)
	$(LD) -o $@ $(LDFLAGS) $^

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
