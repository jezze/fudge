MOD:=$(MODULES_ARCH_PATH)/bga/bga.ko
OBJ:=$(MODULES_ARCH_PATH)/bga/main.o $(MODULES_ARCH_PATH)/bga/driver.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
