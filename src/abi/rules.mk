LIB_ABI:=\
    $(DIR_SRC)/abi/abi.a \

OBJ_ABI:=\

include $(DIR_SRC)/abi/$(ARCH)/rules.mk

$(LIB_ABI): $(OBJ_ABI)
	@echo AR $@ 
	@$(AR) $(ARFLAGS) $@ $^

LIB:=$(LIB) $(LIB_ABI)
OBJ:=$(OBJ) $(OBJ_ABI)
