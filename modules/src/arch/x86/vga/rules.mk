MODULES+=modules/src/arch/x86/vga/vga.ko
CLEAN+=modules/src/arch/x86/vga/main.o modules/src/arch/x86/vga/driver.o

modules/src/arch/x86/vga/vga.ko: lib/memory.o lib/string.o lib/arch/x86/io.o modules/src/arch/x86/vga/main.o modules/src/arch/x86/vga/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
