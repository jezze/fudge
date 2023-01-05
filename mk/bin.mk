$(B): $(O) $(L)
	@echo LD $@
	@$(LD) $(LD_FLAGS) -o $@ $^

BIN:=$(BIN) $(B)
OBJ:=$(OBJ) $(O)
DEP:=$(DEP) $(O:.o=.d)
