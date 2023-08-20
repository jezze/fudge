$(L): $(O)
	@echo AR $@
	@$(AR) $(AR_FLAGS) -o $@ $^

LIB:=$(LIB) $(L)
OBJ:=$(OBJ) $(O)
DEP:=$(DEP) $(O:.o=.d)
