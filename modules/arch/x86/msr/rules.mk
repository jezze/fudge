MODULES+=modules/arch/x86/msr/msr.ko
MODULES_OBJECTS+=modules/arch/x86/msr/main.o modules/arch/x86/msr/msr.o

modules/arch/x86/msr/msr.ko: modules/arch/x86/msr/main.o modules/arch/x86/msr/msr.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
