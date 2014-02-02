MOD:=$(MODULES_PATH)/base/base.ko
OBJ:=$(MODULES_PATH)/base/main.o $(MODULES_PATH)/base/block.o $(MODULES_PATH)/base/clock.o $(MODULES_PATH)/base/keyboard.o $(MODULES_PATH)/base/mouse.o $(MODULES_PATH)/base/network.o $(MODULES_PATH)/base/terminal.o $(MODULES_PATH)/base/timer.o $(MODULES_PATH)/base/video.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
