BIN_MD5:=\
    $(DIR_SRC)/md5/md5 \

OBJ_MD5:=\
    $(DIR_SRC)/md5/md5.o \

DEP_MD5:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \
    $(DIR_SRC)/format/format.a \

$(BIN_MD5): $(OBJ_MD5) $(DEP_MD5)
	@echo LD $@: $^
	@$(LD) $(LDFLAGS) -o $@ $^

BIN:=$(BIN) $(BIN_MD5)
OBJ:=$(OBJ) $(OBJ_MD5)
