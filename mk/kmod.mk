$(M): $(O) $(L)
	@echo LD $@
	@$(LD) $(LD_FLAGS_KMOD) -o $@ $^

KMOD:=$(KMOD) $(M)
KMAP:=$(KMAP) $(N)
OBJ:=$(OBJ) $(O)
DEP:=$(DEP) $(O:.o=.d)
