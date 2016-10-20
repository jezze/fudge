$(DIR_SRC)/%.i: $(DIR_SRC)/%.c
	@echo PP $@: $^
	@$(CC) $(CFLAGS) -E -o $@ -I$(DIR_INCLUDE) -I$(DIR_SRC) $^

$(DIR_SRC)/%.s: $(DIR_SRC)/%.i
	@echo CC $@: $^
	@$(CC) $(CFLAGS) -c -S -o $@ $^

$(DIR_SRC)/%.o: $(DIR_SRC)/%.s
	@echo AS $@: $^
	@$(AS) $(ASFLAGS) -c -o $@ $^
