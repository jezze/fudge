$(B): $(O) $(L)
	@echo LD $@
	@$(LD_BIN) $@ $^

BIN:=$(BIN) $(B)
OBJ:=$(OBJ) $(O)
