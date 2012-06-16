BUILD+=modules/arch/x86/i915/i915.ko
CLEAN+=$(OBJ_modules/arch/x86/i915/i915.ko) modules/arch/x86/i915/i915.ko

OBJ_modules/arch/x86/i915/i915.ko:=modules/arch/x86/i915/main.o modules/arch/x86/i915/driver.o

modules/arch/x86/i915/i915.ko: lib/memory.o lib/string.o lib/arch/x86/io.o $(OBJ_modules/arch/x86/i915/i915.ko)
	$(LD) $(LDFLAGS) -o $@ $^
