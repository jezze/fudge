$(L): $(O)
	@echo AR $@
	@$(AR) $@ $^

LIB:=$(LIB) $(L)
OBJ:=$(OBJ) $(O)
DEP:=$(DEP) $(O:.o=.d)
