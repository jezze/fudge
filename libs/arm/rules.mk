LIBARM:=libs/arm/libarm.a
LIBARM_OBJECTS:=libs/arm/arch.o libs/arm/cpu.o libs/arm/init.o libs/arm/kmi.o libs/arm/reg.o libs/arm/uart.o

$(LIBARM): $(LIBARM_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

LIBS+=$(LIBARM)
LIBS_OBJECTS+=$(LIBARM_OBJECTS)
