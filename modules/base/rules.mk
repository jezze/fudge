MOD_$(DIR):=$(DIR)/base.ko
OBJ_$(DIR):=$(DIR)/main.o $(DIR)/block.o $(DIR)/clock.o $(DIR)/keyboard.o $(DIR)/mouse.o $(DIR)/network.o $(DIR)/terminal.o $(DIR)/timer.o $(DIR)/video.o

$(MOD_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $^ $(LDFLAGS)

MODULES:=$(MODULES) $(MOD_$(DIR))
CLEAN:=$(CLEAN) $(MOD_$(DIR)) $(OBJ_$(DIR))
