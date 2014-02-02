MOD:=$(MODULES_ARCH_PATH)/acpi/acpi.ko
OBJ:=$(MODULES_ARCH_PATH)/acpi/main.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
