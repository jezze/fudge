MOD:=$(MODULES_ARCH_PATH)/uart/uart.ko
OBJ:=$(MODULES_ARCH_PATH)/uart/main.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
