$(B): $(O) $(L)
	@echo LD $@
	@$(LD) -static -o $@ $^

BIN:=$(BIN) $(B)
OBJ:=$(OBJ) $(O)
