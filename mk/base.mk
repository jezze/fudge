%.d: %.s
	@echo DEP $@
	@echo $*.o: > $*.d

%.d: %.c
	@echo DEP $@
	@$(DEP) $*.o $*.c > $*.d

%.i: %.c
	@echo PP $@
	@$(PP) $@ $^

%.s: %.i
	@echo CC $@
	@$(CC) $@ $^

%.o: %.s
	@echo AS $@
	@$(AS) $@ $^
