BUILD+=modules/arch/x86/vga/vga.ko
CLEAN+=$(OBJ_modules/arch/x86/vga/vga.ko) modules/arch/x86/vga/vga.ko

OBJ_modules/arch/x86/vga/vga.ko:=modules/arch/x86/vga/main.o modules/arch/x86/vga/driver.o

modules/arch/x86/vga/vga.ko: GCCFLAGS+=-Iinclude
modules/arch/x86/vga/vga.ko: LDFLAGS+=-Tmodules/linker.ld -r lib/memory.o lib/string.o lib/arch/x86/io.o
modules/arch/x86/vga/vga.ko: lib $(OBJ_modules/arch/x86/vga/vga.ko)
	$(LD) $(LDFLAGS) -o $@ $(OBJ_modules/arch/x86/vga/vga.ko)

