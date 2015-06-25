BIN_$(DIR):=$(DIR)/wnull
OBJ_$(DIR):=$(DIR)/wnull.o $(SRC_PATH)/video/video.o

$(BIN_$(DIR)): $(OBJ_$(DIR)) $(SRC_DEPS)
	$(LD) -o $@ $(LDFLAGS) $^

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
