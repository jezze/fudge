MOD:=$(MODULES_ARCH_PATH)/atapi/atapi.ko
OBJ:=$(MODULES_ARCH_PATH)/atapi/main.o $(MODULES_ARCH_PATH)/atapi/driver.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
