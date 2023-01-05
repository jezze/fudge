$(L): $(O)
	@echo AR $@
	@$(AR) $(AR_FLAGS) $@ $^

LIB:=$(LIB) $(L)
OBJ:=$(OBJ) $(O)
DEP:=$(DEP) $(O:.o=.d)
