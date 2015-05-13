BIN_$(DIR):=$(DIR)/cpio
OBJ_$(DIR):=$(DIR)/cpio.o

$(BIN_$(DIR)): $(OBJ_$(DIR)) $(PACKAGES_DEPS) $(LIBS_PATH)/cpio/libcpio.a
	$(LD) -o $@ $(LDFLAGS) $^

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
