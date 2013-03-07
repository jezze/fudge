MOD:=modules/arch/x86/rtc/rtc.ko
OBJ:=modules/arch/x86/rtc/main.o modules/arch/x86/rtc/device.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
