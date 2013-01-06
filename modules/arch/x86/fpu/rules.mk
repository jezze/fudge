MODULES+=modules/arch/x86/fpu/fpu.ko
MODULES_OBJECTS+=modules/arch/x86/fpu/main.o modules/arch/x86/fpu/fpu.o

modules/arch/x86/fpu/fpu.ko: modules/arch/x86/fpu/main.o modules/arch/x86/fpu/fpu.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
