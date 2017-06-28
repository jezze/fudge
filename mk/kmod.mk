$(M): $(O) $(L)
	@echo LD $@
	@$(LD_KMOD) $@ $^

KMOD:=$(KMOD) $(M)
OBJ:=$(OBJ) $(O)
