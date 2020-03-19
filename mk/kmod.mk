$(M): $(O) $(L)
	@echo LD $@
	@$(LD_KMOD) $@ $^

KMOD:=$(KMOD) $(M)
KMAP:=$(KMAP) $(N)
OBJ:=$(OBJ) $(O)
DEP:=$(DEP) $(O:.o=.d)
