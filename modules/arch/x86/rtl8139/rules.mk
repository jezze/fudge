MODULES+=modules/arch/x86/rtl8139/rtl8139.ko
MODULES_OBJECTS+=modules/arch/x86/rtl8139/main.o modules/arch/x86/rtl8139/driver.o

modules/arch/x86/rtl8139/rtl8139.ko: modules/arch/x86/rtl8139/main.o modules/arch/x86/rtl8139/driver.o $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^
