BIN_$(DIR):=$(DIR)/mount
OBJ_$(DIR):=$(DIR)/mount.o

$(BIN_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $^ $(LDFLAGS)

PACKAGES_BUILD:=$(PACKAGES_BUILD) $(BIN_$(DIR))
PACKAGES_CLEAN:=$(PACKAGES_CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
