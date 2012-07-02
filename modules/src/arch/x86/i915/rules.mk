MODULES+=modules/src/arch/x86/i915/i915.ko
CLEAN+=modules/src/arch/x86/i915/main.o modules/src/arch/x86/i915/driver.o

modules/src/arch/x86/i915/i915.ko: lib/memory.o lib/string.o lib/arch/x86/io.o modules/src/arch/x86/i915/main.o modules/src/arch/x86/i915/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
