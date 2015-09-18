BIN_WM:=\
    $(SRC_PATH)/wm/wm \

OBJ_WM:=\
    $(SRC_PATH)/wm/wm.o \
    $(SRC_PATH)/wm/box.o \
    $(SRC_PATH)/wm/client.o \
    $(SRC_PATH)/wm/draw.o \
    $(SRC_PATH)/wm/image.o \
    $(SRC_PATH)/wm/mouse.o \
    $(SRC_PATH)/wm/panel.o \
    $(SRC_PATH)/wm/renderable.o \
    $(SRC_PATH)/wm/send.o \
    $(SRC_PATH)/wm/text.o \
    $(SRC_PATH)/wm/view.o \
    $(SRC_PATH)/wm/window.o \
    $(SRC_PATH)/video/video.o \
    $(SRC_PATH)/format/pcf.o \

$(BIN_WM): $(OBJ_WM) $(SRC_DEPS)
	$(LD) -o $@ $(LDFLAGS) $^

BIN_WNULL:=\
    $(SRC_PATH)/wm/wnull \

OBJ_WNULL:=\
    $(SRC_PATH)/wm/wnull.o \
    $(SRC_PATH)/wm/box.o \
    $(SRC_PATH)/wm/draw.o \
    $(SRC_PATH)/wm/send.o \
    $(SRC_PATH)/video/video.o \

$(BIN_WNULL): $(OBJ_WNULL) $(SRC_DEPS)
	$(LD) -o $@ $(LDFLAGS) $^

BINS:=$(BINS) $(BIN_WM) $(BIN_WNULL)
CLEAN:=$(CLEAN) $(BIN_WM) $(BIN_WNULL) $(OBJ_WM) $(OBJ_WNULL)
