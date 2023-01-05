L:=\
    $(DIR_LIB)/math/math.a \

O:=\
    $(DIR_LIB)/math/math.o \

$(DIR_LIB)/math/math.o: CC_FLAGS:=$(filter-out -msoft-float, $(CC_FLAGS))

include $(DIR_MK)/lib.mk
