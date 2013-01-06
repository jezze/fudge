MODULES+=modules/arch/x86/bga/bga.ko
MODULES_OBJECTS+=modules/arch/x86/bga/main.o modules/arch/x86/bga/driver.o

modules/arch/x86/bga/bga.ko: modules/arch/x86/bga/main.o modules/arch/x86/bga/driver.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
