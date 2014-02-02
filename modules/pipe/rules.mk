MOD:=$(MODULES_PATH)/pipe/pipe.ko
OBJ:=$(MODULES_PATH)/pipe/main.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
