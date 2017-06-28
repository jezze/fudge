$(M): $(O) $(L)
	@echo LD $@
	@$(LD_KMOD) $@ $^

MOD:=$(MOD) $(M)
OBJ:=$(OBJ) $(O)
