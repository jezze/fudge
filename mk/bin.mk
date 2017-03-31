$(B): $(O) $(L)
	@echo LD $@
	@$(TARGET)-ld -static -o $@ $^

BIN:=$(BIN) $(B)
OBJ:=$(OBJ) $(O)
