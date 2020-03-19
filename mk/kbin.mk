$(B): $(O) $(L)
	@echo LD $@
	@$(LD_KBIN) $@ $^

KBIN:=$(KBIN) $(B)
KMAP:=$(KMAP) $(N)
OBJ:=$(OBJ) $(O)
DEP:=$(DEP) $(O:.o=.d)
