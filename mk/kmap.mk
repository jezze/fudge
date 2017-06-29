$(M): $(B)
	@echo NM $@
	@$(NM) $^ > $@

KMAP:=$(KMAP) $(M)
