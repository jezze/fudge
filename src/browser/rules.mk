B:=\
    $(DIR_SRC)/browser/browser \

O:=\
    $(DIR_SRC)/browser/browser.o \
    $(DIR_SRC)/browser/css.o \
    $(DIR_SRC)/browser/date.o \
    $(DIR_SRC)/browser/font.o \
    $(DIR_SRC)/browser/html.o \
    $(DIR_SRC)/browser/js.o \
    $(DIR_SRC)/browser/layout.o \
    $(DIR_SRC)/browser/number.o \
    $(DIR_SRC)/browser/paint.o \
    $(DIR_SRC)/browser/svg.o \
    $(DIR_SRC)/wutils/kv.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/font/font.a \
    $(DIR_LIB)/image/image.a \
    $(DIR_LIB)/compression/compression.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/hash/hash.a \

$(DIR_SRC)/browser/js.o: CC_FLAGS:=$(filter-out $(CC_FLAGS_FLOAT), $(CC_FLAGS))
$(DIR_SRC)/browser/number.o: CC_FLAGS:=$(filter-out $(CC_FLAGS_FLOAT), $(CC_FLAGS))
$(DIR_SRC)/browser/date.o: CC_FLAGS:=$(filter-out $(CC_FLAGS_FLOAT), $(CC_FLAGS))
$(DIR_SRC)/browser/svg.o: CC_FLAGS:=$(filter-out $(CC_FLAGS_FLOAT), $(CC_FLAGS))

include $(DIR_MK)/bin.mk

ROOTBIN:=$(ROOTBIN) $(B)
