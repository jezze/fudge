$(N): $(B)
	@echo NM $@
	@$(NM) $^ > $@

KMAP:=$(KMAP) $(N)
