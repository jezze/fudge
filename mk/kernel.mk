$(B): $(O) $(L)
	@echo LD $@
	@$(LD) $@ -T$(DIR_SRC)/kernel/$(ARCH)/linker.ld $^

BIN:=$(BIN) $(B)
OBJ:=$(OBJ) $(O)
