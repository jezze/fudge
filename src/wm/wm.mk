BIN_$(DIR):=$(DIR)/wm
OBJ_$(DIR):=$(DIR)/wm.o $(DIR)/box.o $(DIR)/draw.o $(DIR)/mouse.o $(DIR)/panel.o $(DIR)/send.o $(DIR)/view.o $(DIR)/window.o $(SRC_PATH)/video/video.o

$(BIN_$(DIR)): $(OBJ_$(DIR)) $(SRC_DEPS)
	$(LD) -o $@ $(LDFLAGS) $^

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
