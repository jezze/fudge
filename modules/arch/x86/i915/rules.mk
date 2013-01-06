MODULES+=modules/arch/x86/i915/i915.ko
MODULES_OBJECTS+=modules/arch/x86/i915/main.o modules/arch/x86/i915/driver.o

modules/arch/x86/i915/i915.ko: modules/arch/x86/i915/main.o modules/arch/x86/i915/driver.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
