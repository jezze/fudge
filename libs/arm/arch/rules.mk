LIBARCH:=libs/arm/arch/libarch.a
LIBARCH_OBJ:=libs/arm/arch/arch.o libs/arm/arch/cpu.o libs/arm/arch/init.o libs/arm/arch/kmi.o libs/arm/arch/reg.o libs/arm/arch/uart.o

$(LIBARCH): $(LIBARCH_OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBS+=$(LIBARCH)
LIBS_OBJECTS+=$(LIBARCH_OBJ)
