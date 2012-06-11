BUILD+=modules/arch/x86/uart/uart.ko
CLEAN+=$(OBJ_modules/arch/x86/uart/uart.ko) modules/arch/x86/uart/uart.ko

OBJ_modules/arch/x86/uart/uart.ko:=modules/arch/x86/uart/main.o modules/arch/x86/uart/driver.o modules/arch/x86/uart/device.o

modules/arch/x86/uart/uart.ko: GCCFLAGS+=-Iinclude
modules/arch/x86/uart/uart.ko: LDFLAGS+=-Tmodules/linker.ld -r lib/memory.o lib/string.o lib/arch/x86/io.o
modules/arch/x86/uart/uart.ko: lib $(OBJ_modules/arch/x86/uart/uart.ko)
	$(LD) $(LDFLAGS) -o $@ $(OBJ_modules/arch/x86/uart/uart.ko)

