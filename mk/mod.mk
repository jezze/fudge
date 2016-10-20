$(M): LDFLAGS+=-T$(DIR_SRC)/modules/linker.ld -r
$(M): $(O) $(L)
	@echo LD $@
	@$(LD) $(LDFLAGS) -o $@ $^

MOD:=$(MOD) $(M)
OBJ:=$(OBJ) $(O)
