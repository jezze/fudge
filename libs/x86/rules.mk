LIB:=libs/x86/libx86.a
OBJ:=libs/x86/arch.o
OBJ+=libs/x86/archs.o
OBJ+=libs/x86/cpu.o
OBJ+=libs/x86/gdt.o
OBJ+=libs/x86/idt.o
OBJ+=libs/x86/mmu.o
OBJ+=libs/x86/multi.o
OBJ+=libs/x86/tss.o

$(LIB): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

LIBX86:=$(LIB)
LIBS+=$(LIB)
LIBS_OBJECTS+=$(OBJ)
