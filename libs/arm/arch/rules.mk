LIBARCH:=$(LIBS_PATH)/arm/arch/libarch.a
LIBARCH_OBJ:=$(LIBS_PATH)/arm/arch/arch.o $(LIBS_PATH)/arm/arch/cpu.o $(LIBS_PATH)/arm/arch/init.o $(LIBS_PATH)/arm/arch/kmi.o $(LIBS_PATH)/arm/arch/reg.o $(LIBS_PATH)/arm/arch/uart.o

$(LIBARCH): $(LIBARCH_OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBS+=$(LIBARCH)
LIBS_OBJECTS+=$(LIBARCH_OBJ)
