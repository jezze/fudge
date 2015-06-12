BIN_$(DIR):=$(DIR)/modload
OBJ_$(DIR):=$(DIR)/modload.o $(SRC_PATH)/elf/elf.o

$(BIN_$(DIR)): $(OBJ_$(DIR)) $(SRC_DEPS)
	$(LD) -o $@ $(LDFLAGS) $^

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
