$(M): $(O) $(L)
	@echo LD $@
	@$(LD) $@ -T$(DIR_SRC)/modules/linker.ld -r $^

MOD:=$(MOD) $(M)
OBJ:=$(OBJ) $(O)
