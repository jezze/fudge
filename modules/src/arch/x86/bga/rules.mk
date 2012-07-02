MODULES+=modules/src/arch/x86/bga/bga.ko
CLEAN+=modules/src/arch/x86/bga/main.o modules/src/arch/x86/bga/driver.o

modules/src/arch/x86/bga/bga.ko: lib/memory.o lib/string.o lib/arch/x86/io.o modules/src/arch/x86/bga/main.o modules/src/arch/x86/bga/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
