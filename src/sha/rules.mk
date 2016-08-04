BIN_SHA1:=\
    $(DIR_SRC)/sha/sha1 \

OBJ_SHA1:=\
    $(DIR_SRC)/sha/sha1.o \

DEP_SHA1:=\
    $(DIR_SRC)/abi/abi.a \
    $(DIR_SRC)/fudge/fudge.a \
    $(DIR_SRC)/format/format.a \

$(BIN_SHA1): $(OBJ_SHA1) $(DEP_SHA1)
	@echo LD $@: $^
	@$(LD) $(LDFLAGS) -o $@ $^

BIN:=$(BIN) $(BIN_SHA1)
OBJ:=$(OBJ) $(OBJ_SHA1)
