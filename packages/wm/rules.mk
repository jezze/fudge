BIN_$(DIR):=$(DIR)/wm
OBJ_$(DIR):=$(DIR)/wm.o $(DIR)/box.o $(DIR)/mouse.o $(DIR)/panel.o $(DIR)/view.o $(DIR)/window.o $(PACKAGES_PATH)/video/video.o

$(BIN_$(DIR)): $(OBJ_$(DIR)) $(PACKAGES_DEPS)
	$(LD) -o $@ $(LDFLAGS) $^

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
