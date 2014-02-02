MOD:=$(MODULES_ARCH_PATH)/apic/apic.ko
OBJ:=$(MODULES_ARCH_PATH)/apic/main.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
