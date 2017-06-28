$(B): $(O) $(L)
	@echo LD $@
	@$(LD_KBIN) $@ $^

KBIN:=$(KBIN) $(B)
OBJ:=$(OBJ) $(O)
