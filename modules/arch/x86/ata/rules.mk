BUILD+=modules/arch/x86/ata/ata.ko
CLEAN+=$(OBJ_modules/arch/x86/ata/ata.ko) modules/arch/x86/ata/ata.ko

OBJ_modules/arch/x86/ata/ata.ko:=modules/arch/x86/ata/main.o modules/arch/x86/ata/bus.o modules/arch/x86/ata/device.o

modules/arch/x86/ata/ata.ko: GCCFLAGS+=-Iinclude
modules/arch/x86/ata/ata.ko: LDFLAGS+=-Tmodules/linker.ld -r lib/memory.o lib/string.o lib/arch/x86/io.o
modules/arch/x86/ata/ata.ko: lib $(OBJ_modules/arch/x86/ata/ata.ko)
	$(LD) $(LDFLAGS) -o $@ $(OBJ_modules/arch/x86/ata/ata.ko)

