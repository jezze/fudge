MODULES+=modules/src/arch/x86/i915/i915.ko
CLEAN+=modules/src/arch/x86/i915/main.o modules/src/arch/x86/i915/driver.o

modules/src/arch/x86/i915/i915.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/i915/main.o modules/src/arch/x86/i915/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
