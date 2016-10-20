$(L): $(O)
	@echo AR $@ 
	@$(AR) $(ARFLAGS) $@ $^

LIB:=$(LIB) $(L)
OBJ:=$(OBJ) $(O)
