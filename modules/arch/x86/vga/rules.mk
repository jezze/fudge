MOD:=$(MODULES_ARCH_PATH)/vga/vga.ko
OBJ:=$(MODULES_ARCH_PATH)/vga/main.o $(MODULES_ARCH_PATH)/vga/driver.o $(MODULES_ARCH_PATH)/vga/timing.o $(MODULES_ARCH_PATH)/vga/registers.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
