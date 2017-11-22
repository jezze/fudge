%.d: %.s
	@echo DEP $@
	@echo $*.o: > $*.d

%.d: %.c
	@echo DEP $@
	@$(CROSS_COMPILE)gcc -MM -MT $*.o $*.c > $*.d

%.i: %.c
	@echo PP $@
	@$(PP) $@ $^

%.s: %.i
	@echo CC $@
	@$(CC) $@ $^

%.o: %.s
	@echo AS $@
	@$(AS) $@ $^
