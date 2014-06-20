LIB_$(DIR):=$(DIR)/libarch.a
OBJ_$(DIR):=$(DIR)/arch.o $(DIR)/archs.o $(DIR)/cpu.o $(DIR)/gdt.o $(DIR)/idt.o $(DIR)/mmu.o $(DIR)/tss.o

$(LIB_$(DIR)): $(OBJ_$(DIR))
	$(AR) $(ARFLAGS) $@ $^

LIBS_BUILD:=$(LIBS_BUILD) $(LIB_$(DIR))
LIBS_CLEAN:=$(LIBS_CLEAN) $(LIB_$(DIR)) $(OBJ_$(DIR))
