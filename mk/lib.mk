$(L): $(O)
	@echo AR $@
	@$(AR) $@ $^

LIB:=$(LIB) $(L)
OBJ:=$(OBJ) $(O)
