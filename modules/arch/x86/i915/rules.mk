MODULES+=modules/arch/x86/i915/i915.ko
CLEAN+=modules/arch/x86/i915/main.o modules/arch/x86/i915/driver.o

modules/arch/x86/i915/i915.ko: lib/memory.o lib/string.o lib/arch/x86/io.o modules/arch/x86/i915/main.o modules/arch/x86/i915/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
