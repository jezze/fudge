$(B): LDFLAGS:=$(LDFLAGS) -Tplatform/$(PLATFORM)/linker.ld
$(B): $(O) $(L)
	@echo LD $@
	@$(LD) $(LDFLAGS) -o $@ $^

BIN:=$(BIN) $(B)
OBJ:=$(OBJ) $(O)
