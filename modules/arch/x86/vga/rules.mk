MODULES+=modules/arch/x86/vga/vga.ko
CLEAN+=modules/arch/x86/vga/main.o modules/arch/x86/vga/driver.o

modules/arch/x86/vga/vga.ko: lib/memory.o lib/string.o lib/arch/x86/io.o modules/arch/x86/vga/main.o modules/arch/x86/vga/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
