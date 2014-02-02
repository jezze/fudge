MOD:=$(MODULES_PATH)/log/log.ko
OBJ:=$(MODULES_PATH)/log/main.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
