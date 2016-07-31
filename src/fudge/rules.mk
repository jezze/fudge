LIB_FUDGE:=\
    $(DIR_SRC)/fudge/fudge.a \

OBJ_FUDGE:=\
    $(DIR_SRC)/fudge/ascii.o \
    $(DIR_SRC)/fudge/buffer.o \
    $(DIR_SRC)/fudge/ctrl.o \
    $(DIR_SRC)/fudge/list.o \
    $(DIR_SRC)/fudge/memory.o \

$(LIB_FUDGE): $(OBJ_FUDGE)
	@echo AR $@ 
	@$(AR) $(ARFLAGS) $@ $^

LIB:=$(LIB) $(LIB_FUDGE)
OBJ:=$(OBJ) $(OBJ_FUDGE)
