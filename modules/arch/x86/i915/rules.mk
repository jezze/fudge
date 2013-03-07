MOD:=modules/arch/x86/i915/i915.ko
OBJ:=modules/arch/x86/i915/main.o modules/arch/x86/i915/driver.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
