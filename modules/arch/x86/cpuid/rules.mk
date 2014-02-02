MOD:=$(MODULES_ARCH_PATH)/cpuid/cpuid.ko
OBJ:=$(MODULES_ARCH_PATH)/cpuid/main.o $(MODULES_ARCH_PATH)/cpuid/cpuid.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
