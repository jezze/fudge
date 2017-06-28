$(B): $(O) $(L)
	@echo LD $@
	@$(LD_KBIN) $@ $^

BIN:=$(BIN) $(B)
OBJ:=$(OBJ) $(O)
