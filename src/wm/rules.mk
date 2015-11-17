BIN_WM:=\
    $(SRC_PATH)/wm/wm \

OBJ_WM:=\
    $(SRC_PATH)/wm/wm.o \
    $(SRC_PATH)/wm/box.o \
    $(SRC_PATH)/wm/element.o \
    $(SRC_PATH)/wm/send.o \
    $(SRC_PATH)/lib/video.o \

$(BIN_WM): $(OBJ_WM) $(SRC_DEPS)
	$(LD) -o $@ $(LDFLAGS) $^

BIN_WDRAW:=\
    $(SRC_PATH)/wm/wdraw \

OBJ_WDRAW:=\
    $(SRC_PATH)/wm/wdraw.o \
    $(SRC_PATH)/wm/box.o \
    $(SRC_PATH)/wm/element.o \
    $(SRC_PATH)/lib/video.o \
    $(SRC_PATH)/format/pcf.o \

$(BIN_WDRAW): $(OBJ_WDRAW) $(SRC_DEPS)
	$(LD) -o $@ $(LDFLAGS) $^

BIN_WTEXT:=\
    $(SRC_PATH)/wm/wtext \

OBJ_WTEXT:=\
    $(SRC_PATH)/wm/wtext.o \
    $(SRC_PATH)/wm/box.o \
    $(SRC_PATH)/wm/element.o \

$(BIN_WTEXT): $(OBJ_WTEXT) $(SRC_DEPS)
	$(LD) -o $@ $(LDFLAGS) $^

BIN_WNULL:=\
    $(SRC_PATH)/wm/wnull \

OBJ_WNULL:=\
    $(SRC_PATH)/wm/wnull.o \
    $(SRC_PATH)/wm/box.o \
    $(SRC_PATH)/wm/element.o \
    $(SRC_PATH)/wm/send.o \
    $(SRC_PATH)/wm/keymap.o \

$(BIN_WNULL): $(OBJ_WNULL) $(SRC_DEPS)
	$(LD) -o $@ $(LDFLAGS) $^

BINS:=$(BINS) $(BIN_WM) $(BIN_WDRAW) $(BIN_WTEXT) $(BIN_WNULL)
CLEAN:=$(CLEAN) $(BIN_WM) $(BIN_WDRAW) $(BIN_WTEXT) $(BIN_WNULL) $(OBJ_WM) $(OBJ_WDRAW) $(OBJ_WTEXT) $(OBJ_WNULL)
