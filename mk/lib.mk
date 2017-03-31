$(L): $(O)
	@echo AR $@ 
	@$(TARGET)-ar rcs $@ $^

LIB:=$(LIB) $(L)
OBJ:=$(OBJ) $(O)
