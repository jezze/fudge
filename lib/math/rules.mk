L:=\
    $(DIR_LIB)/math/math.a \

O:=\
    $(DIR_LIB)/math/math.o \

$(DIR_LIB)/math/math.o: CC_FLAGS:=$(filter-out $(CC_FLAGS_FLOAT), $(CC_FLAGS))

include $(DIR_MK)/lib.mk
