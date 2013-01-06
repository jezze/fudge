MODULES+=modules/arch/x86/vga/vga.ko
MODULES_OBJECTS+=modules/arch/x86/vga/main.o modules/arch/x86/vga/driver.o

modules/arch/x86/vga/vga.ko: modules/arch/x86/vga/main.o modules/arch/x86/vga/driver.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
