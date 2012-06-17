MODULES+=modules/arch/x86/vga/vga.ko
CLEAN+=$(OBJ_modules/arch/x86/vga/vga.ko) modules/arch/x86/vga/vga.ko

OBJ_modules/arch/x86/vga/vga.ko:=modules/arch/x86/vga/main.o modules/arch/x86/vga/driver.o

modules/arch/x86/vga/vga.ko: lib/memory.o lib/string.o lib/arch/x86/io.o $(OBJ_modules/arch/x86/vga/vga.ko)
	$(LD) $(LDFLAGS) -o $@ $^
