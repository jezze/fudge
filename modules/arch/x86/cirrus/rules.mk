MOD:=$(MODULES_ARCH_PATH)/cirrus/cirrus.ko
OBJ:=$(MODULES_ARCH_PATH)/cirrus/main.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
