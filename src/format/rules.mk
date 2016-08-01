LIB_FORMAT:=\
    $(DIR_SRC)/format/format.a \

OBJ_FORMAT:=\
    $(DIR_SRC)/format/cpio.o \
    $(DIR_SRC)/format/md5.o \
    $(DIR_SRC)/format/pcf.o \
    $(DIR_SRC)/format/sha1.o \
    $(DIR_SRC)/format/tar.o \
    $(DIR_SRC)/format/utf8.o \

$(LIB_FORMAT): $(OBJ_FORMAT)
	@echo AR $@ 
	@$(AR) $(ARFLAGS) $@ $^

LIB:=$(LIB) $(LIB_FORMAT)
OBJ:=$(OBJ) $(OBJ_FORMAT)
