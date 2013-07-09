LIBX86:=libs/x86/libx86.a
LIBX86_OBJECTS:=libs/x86/arch.o libs/x86/archs.o libs/x86/cpu.o libs/x86/gdt.o libs/x86/idt.o libs/x86/mmu.o libs/x86/multi.o libs/x86/tss.o

$(LIBX86): $(LIBX86_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

LIBS+=$(LIBX86)
LIBS_OBJECTS+=$(LIBX86_OBJECTS)
