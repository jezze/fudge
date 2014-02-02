MOD:=$(MODULES_ARCH_PATH)/pit/pit.ko
OBJ:=$(MODULES_ARCH_PATH)/pit/main.o $(MODULES_ARCH_PATH)/pit/driver.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
