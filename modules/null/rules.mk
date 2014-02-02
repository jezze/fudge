MOD:=$(MODULES_PATH)/null/null.ko
OBJ:=$(MODULES_PATH)/null/main.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
