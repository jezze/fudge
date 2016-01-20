BIN_WDRAW:=\
    $(DIR_SRC)/wm/wdraw \

OBJ_WDRAW:=\
    $(DIR_SRC)/wm/wdraw.o \
    $(DIR_SRC)/wm/box.o \
    $(DIR_SRC)/wm/element.o \
    $(DIR_SRC)/fudge/ascii.o \
    $(DIR_SRC)/fudge/ctrl.o \
    $(DIR_SRC)/fudge/memory.o \
    $(DIR_SRC)/lib/file.o \
    $(DIR_SRC)/format/pcf.o \

$(BIN_WDRAW): $(OBJ_WDRAW) $(OBJ_STD) $(OBJ_ABI)
	$(LD) -o $@ $(LDFLAGS) $^

BIN_WECHO:=\
    $(DIR_SRC)/wm/wecho \

OBJ_WECHO:=\
    $(DIR_SRC)/wm/wecho.o \
    $(DIR_SRC)/wm/box.o \
    $(DIR_SRC)/wm/element.o \
    $(DIR_SRC)/wm/send.o \
    $(DIR_SRC)/fudge/ascii.o \
    $(DIR_SRC)/fudge/memory.o \
    $(DIR_SRC)/lib/file.o \

$(BIN_WECHO): $(OBJ_WECHO) $(OBJ_STD) $(OBJ_ABI)
	$(LD) -o $@ $(LDFLAGS) $^

BIN_WM:=\
    $(DIR_SRC)/wm/wm \

OBJ_WM:=\
    $(DIR_SRC)/wm/wm.o \
    $(DIR_SRC)/wm/box.o \
    $(DIR_SRC)/wm/element.o \
    $(DIR_SRC)/wm/send.o \
    $(DIR_SRC)/fudge/list.o \
    $(DIR_SRC)/fudge/memory.o \
    $(DIR_SRC)/lib/file.o \

$(BIN_WM): $(OBJ_WM) $(OBJ_STD) $(OBJ_ABI)
	$(LD) -o $@ $(LDFLAGS) $^

BIN_WSHELL:=\
    $(DIR_SRC)/wm/wshell \

OBJ_WSHELL:=\
    $(DIR_SRC)/wm/wshell.o \
    $(DIR_SRC)/wm/box.o \
    $(DIR_SRC)/wm/element.o \
    $(DIR_SRC)/wm/send.o \
    $(DIR_SRC)/wm/keymap.o \
    $(DIR_SRC)/fudge/buffer.o \
    $(DIR_SRC)/fudge/memory.o \
    $(DIR_SRC)/lib/file.o \

$(BIN_WSHELL): $(OBJ_WSHELL) $(OBJ_STD) $(OBJ_ABI)
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

BIN:=$(BIN) $(BIN_WDRAW) $(BIN_WECHO) $(BIN_WM) $(BIN_WSHELL) $(BIN_WTEXT)
OBJ:=$(OBJ) $(OBJ_WDRAW) $(OBJ_WECHO) $(OBJ_WM) $(OBJ_WSHELL) $(OBJ_WTEXT)
