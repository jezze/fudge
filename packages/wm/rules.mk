BIN_$(DIR):=$(BUILD_BIN)/wm
OBJ_$(DIR):=$(DIR)/wm.o $(DIR)/box.o $(DIR)/mouse.o $(DIR)/panel.o $(DIR)/text.o $(DIR)/video.o $(DIR)/view.o $(DIR)/window.o

$(BIN_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) $^ $(PACKAGES_LDFLAGS)

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
