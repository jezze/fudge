MOD:=$(MODULES_ARCH_PATH)/ps2/ps2.ko
OBJ:=$(MODULES_ARCH_PATH)/ps2/main.o $(MODULES_ARCH_PATH)/ps2/keyboard_driver.o $(MODULES_ARCH_PATH)/ps2/mouse_driver.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
