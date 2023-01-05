B:=\
    $(DIR_SRC)/demo/demo \

O:=\
    $(DIR_SRC)/demo/demo.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/math/math.a \

$(DIR_SRC)/demo/demo.o: CC_FLAGS:=$(filter-out -msoft-float, $(CC_FLAGS))

include $(DIR_MK)/bin.mk
