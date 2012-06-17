MODULES+=modules/arch/x86/rtl8139/rtl8139.ko
CLEAN+=modules/arch/x86/rtl8139/main.o modules/arch/x86/rtl8139/driver.o

modules/arch/x86/rtl8139/rtl8139.ko: lib/memory.o lib/string.o lib/arch/x86/io.o modules/arch/x86/rtl8139/main.o modules/arch/x86/rtl8139/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
