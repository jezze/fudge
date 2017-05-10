$(B): $(O) $(L)
	@echo LD $@
	@$(LD) $@ $^

BIN:=$(BIN) $(B)
OBJ:=$(OBJ) $(O)
