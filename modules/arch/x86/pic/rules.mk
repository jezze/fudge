MODULES+=modules/arch/x86/pic/pic.ko
MODULES_OBJECTS+=modules/arch/x86/pic/main.o modules/arch/x86/pic/pic.o

modules/arch/x86/pic/pic.ko: modules/arch/x86/pic/main.o modules/arch/x86/pic/pic.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
