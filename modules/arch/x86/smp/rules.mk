MODULES+=modules/arch/x86/smp/smp.ko
MODULES_OBJECTS+=modules/arch/x86/smp/main.o

modules/arch/x86/smp/smp.ko: modules/arch/x86/smp/main.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
