$(B): $(O) $(D)
	@echo LD $@: $^
	@$(LD) $(LDFLAGS) -o $@ $^

BIN:=$(BIN) $(B)
OBJ:=$(OBJ) $(O)
