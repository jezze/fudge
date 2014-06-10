MOD:=$(MODULES_ARCH_PATH)/platform/platform.ko
OBJ:=$(MODULES_ARCH_PATH)/platform/main.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
