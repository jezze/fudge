B:=\
    $(DIR_SRC)/demo/demo \

O:=\
    $(DIR_SRC)/demo/demo.o \
    $(DIR_SRC)/demo/vector2.o \
    $(DIR_SRC)/demo/vector3.o \
    $(DIR_SRC)/demo/matrix3x3.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/math/math.a \

$(DIR_SRC)/demo/demo.o: CC_FLAGS:=$(filter-out -msoft-float, $(CC_FLAGS))
$(DIR_SRC)/demo/vector2.o: CC_FLAGS:=$(filter-out -msoft-float, $(CC_FLAGS))
$(DIR_SRC)/demo/vector3.o: CC_FLAGS:=$(filter-out -msoft-float, $(CC_FLAGS))

include $(DIR_MK)/bin.mk
