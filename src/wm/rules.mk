BIN_WM:=\
    $(DIR_SRC)/wm/wm \

OBJ_WM:=\
    $(DIR_SRC)/wm/wm.o \
    $(DIR_SRC)/wm/box.o \
    $(DIR_SRC)/wm/element.o \
    $(DIR_SRC)/wm/send.o \
    $(DIR_SRC)/fudge/ctrl.o \
    $(DIR_SRC)/fudge/list.o \
    $(DIR_SRC)/fudge/memory.o \
    $(DIR_SRC)/lib/file.o \
    $(DIR_SRC)/lib/video.o \

$(BIN_WM): $(OBJ_WM) $(OBJ_STD) $(OBJ_ABI)
	$(LD) -o $@ $(LDFLAGS) $^

BIN_WDRAW:=\
    $(DIR_SRC)/wm/wdraw \

OBJ_WDRAW:=\
    $(DIR_SRC)/wm/wdraw.o \
    $(DIR_SRC)/wm/box.o \
    $(DIR_SRC)/wm/element.o \
    $(DIR_SRC)/fudge/ctrl.o \
    $(DIR_SRC)/fudge/memory.o \
    $(DIR_SRC)/lib/file.o \
    $(DIR_SRC)/lib/video.o \
    $(DIR_SRC)/format/pcf.o \

$(BIN_WDRAW): $(OBJ_WDRAW) $(OBJ_STD) $(OBJ_ABI)
	$(LD) -o $@ $(LDFLAGS) $^

BIN_WTEXT:=\
    $(DIR_SRC)/wm/wtext \

OBJ_WTEXT:=\
    $(DIR_SRC)/wm/wtext.o \
    $(DIR_SRC)/wm/box.o \
    $(DIR_SRC)/wm/element.o \
    $(DIR_SRC)/fudge/ascii.o \
    $(DIR_SRC)/fudge/memory.o \
    $(DIR_SRC)/lib/file.o \

$(BIN_WTEXT): $(OBJ_WTEXT) $(OBJ_STD) $(OBJ_ABI)
	$(LD) -o $@ $(LDFLAGS) $^

BIN_WNULL:=\
    $(DIR_SRC)/wm/wnull \

OBJ_WNULL:=\
    $(DIR_SRC)/wm/wnull.o \
    $(DIR_SRC)/wm/box.o \
    $(DIR_SRC)/wm/element.o \
    $(DIR_SRC)/wm/send.o \
    $(DIR_SRC)/wm/keymap.o \
    $(DIR_SRC)/fudge/memory.o \
    $(DIR_SRC)/lib/file.o \

$(BIN_WNULL): $(OBJ_WNULL) $(OBJ_STD) $(OBJ_ABI)
	$(LD) -o $@ $(LDFLAGS) $^

BIN:=$(BIN) $(BIN_WM) $(BIN_WDRAW) $(BIN_WTEXT) $(BIN_WNULL)
CLEAN:=$(CLEAN) $(BIN_WM) $(BIN_WDRAW) $(BIN_WTEXT) $(BIN_WNULL) $(OBJ_WM) $(OBJ_WDRAW) $(OBJ_WTEXT) $(OBJ_WNULL)
