$(B): $(O) $(L)
	@echo LD $@
	@$(LD) $(LD_FLAGS_BIN) -o $@ $^

TESTBIN:=$(TESTBIN) $(B)
OBJ:=$(OBJ) $(O)
DEP:=$(DEP) $(O:.o=.d)
