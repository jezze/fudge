MODULES+=modules/src/arch/x86/rtl8139/rtl8139.ko
CLEAN+=modules/src/arch/x86/rtl8139/main.o modules/src/arch/x86/rtl8139/driver.o

modules/src/arch/x86/rtl8139/rtl8139.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/rtl8139/main.o modules/src/arch/x86/rtl8139/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
