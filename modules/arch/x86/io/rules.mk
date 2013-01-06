MODULES+=modules/arch/x86/io/io.ko
MODULES_OBJECTS+=modules/arch/x86/io/main.o modules/arch/x86/io/io.o

modules/arch/x86/io/io.ko: modules/arch/x86/io/main.o modules/arch/x86/io/io.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
