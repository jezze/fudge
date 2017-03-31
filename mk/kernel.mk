$(B): $(O) $(L)
	@echo LD $@
	@$(TARGET)-ld -static -Tplatform/$(PLATFORM)/linker.ld -o $@ $^

BIN:=$(BIN) $(B)
OBJ:=$(OBJ) $(O)
