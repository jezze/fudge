BUILD+=modules/arch/x86/pit/pit.ko
CLEAN+=$(OBJ_modules/arch/x86/pit/pit.ko) modules/arch/x86/pit/pit.ko

OBJ_modules/arch/x86/pit/pit.ko:=modules/arch/x86/pit/main.o modules/arch/x86/pit/driver.o modules/arch/x86/pit/device.o

modules/arch/x86/pit/pit.ko: LDFLAGS+=-Tmodules/linker.ld -r lib/memory.o lib/string.o lib/arch/x86/io.o
modules/arch/x86/pit/pit.ko: lib $(OBJ_modules/arch/x86/pit/pit.ko)
	$(LD) $(LDFLAGS) -o $@ $(OBJ_modules/arch/x86/pit/pit.ko)

