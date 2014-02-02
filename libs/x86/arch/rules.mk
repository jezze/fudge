LIBARCH:=$(LIBS_PATH)/x86/arch/libarch.a
LIBARCH_OBJ:=$(LIBS_PATH)/x86/arch/arch.o $(LIBS_PATH)/x86/arch/archs.o $(LIBS_PATH)/x86/arch/cpu.o $(LIBS_PATH)/x86/arch/gdt.o $(LIBS_PATH)/x86/arch/idt.o $(LIBS_PATH)/x86/arch/mmu.o $(LIBS_PATH)/x86/arch/multi.o $(LIBS_PATH)/x86/arch/tss.o

$(LIBARCH): $(LIBARCH_OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBS+=$(LIBARCH)
LIBS_OBJECTS+=$(LIBARCH_OBJ)
