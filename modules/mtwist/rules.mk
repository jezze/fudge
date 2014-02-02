MOD:=$(MODULES_PATH)/mtwist/mtwist.ko
OBJ:=$(MODULES_PATH)/mtwist/main.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
