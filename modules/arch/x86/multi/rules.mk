MODULES+=modules/arch/x86/multi/multi.ko
MODULES_OBJECTS+=modules/arch/x86/multi/main.o

modules/arch/x86/multi/multi.ko: modules/arch/x86/multi/main.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
