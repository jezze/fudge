$(M): $(O) $(L)
	@echo LD $@
	@$(LD) -static -T$(DIR_SRC)/modules/linker.ld -r -o $@ $^

MOD:=$(MOD) $(M)
OBJ:=$(OBJ) $(O)
