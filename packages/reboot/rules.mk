BIN_$(DIR):=$(DIR)/reboot
OBJ_$(DIR):=$(DIR)/reboot.o

$(BIN_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $^ $(LDFLAGS)

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
