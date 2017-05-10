$(L): $(O)
	@echo AR $@ 
	@$(AR) rcs $@ $^

LIB:=$(LIB) $(L)
OBJ:=$(OBJ) $(O)
