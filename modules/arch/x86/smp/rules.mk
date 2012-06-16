BUILD+=modules/arch/x86/smp/smp.ko
CLEAN+=$(OBJ_modules/arch/x86/smp/smp.ko) modules/arch/x86/smp/smp.ko

OBJ_modules/arch/x86/smp/smp.ko:=modules/arch/x86/smp/main.o modules/arch/x86/smp/driver.o

modules/arch/x86/smp/smp.ko: LDFLAGS+=-Tmodules/linker.ld -r lib/memory.o lib/string.o lib/arch/x86/io.o
modules/arch/x86/smp/smp.ko: lib $(OBJ_modules/arch/x86/smp/smp.ko)
	$(LD) $(LDFLAGS) -o $@ $(OBJ_modules/arch/x86/smp/smp.ko)

