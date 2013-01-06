MODULES+=modules/arch/x86/atapi/atapi.ko
MODULES_OBJECTS+=modules/arch/x86/atapi/main.o modules/arch/x86/atapi/driver.o

modules/arch/x86/atapi/atapi.ko: modules/arch/x86/atapi/main.o modules/arch/x86/atapi/driver.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
