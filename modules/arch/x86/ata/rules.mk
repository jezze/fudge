MOD:=$(MODULES_ARCH_PATH)/ata/ata.ko
OBJ:=$(MODULES_ARCH_PATH)/ata/main.o $(MODULES_ARCH_PATH)/ata/driver.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
