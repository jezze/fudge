B:=\
    $(DIR_SRC)/wm/wm \

O:=\
    $(DIR_SRC)/wm/attr.o \
    $(DIR_SRC)/wm/blit.o \
    $(DIR_SRC)/wm/cmap.o \
    $(DIR_SRC)/wm/parser.o \
    $(DIR_SRC)/wm/pool.o \
    $(DIR_SRC)/wm/render.o \
    $(DIR_SRC)/wm/strpool.o \
    $(DIR_SRC)/wm/text.o \
    $(DIR_SRC)/wm/util.o \
    $(DIR_SRC)/wm/widget.o \
    $(DIR_SRC)/wm/wm.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/image/image.a \

include $(DIR_MK)/bin.mk
