$(M): $(O) $(L)
	@echo LD $@
	@$(TARGET)-ld -static -T$(DIR_SRC)/modules/linker.ld -r -o $@ $^

MOD:=$(MOD) $(M)
OBJ:=$(OBJ) $(O)
