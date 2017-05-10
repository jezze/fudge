$(B): $(O) $(L)
	@echo LD $@
	@$(LD) -static -T$(DIR_SRC)/kernel/$(ARCH)/linker.ld -o $@ $^

BIN:=$(BIN) $(B)
OBJ:=$(OBJ) $(O)
