LIB_STD:=\
    $(DIR_SRC)/std/std.a \

OBJ_STD:=\

include $(DIR_SRC)/std/$(ARCH)/rules.mk

$(LIB_STD): $(OBJ_STD)
	@echo AR $@ 
	@$(AR) $(ARFLAGS) $@ $^

LIB:=$(LIB) $(LIB_STD)
OBJ:=$(OBJ) $(OBJ_STD)
