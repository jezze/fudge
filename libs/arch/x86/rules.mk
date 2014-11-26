LIB_$(DIR):=$(BUILD_LIB)/libarch.a
OBJ_$(DIR):=$(DIR)/arch.o $(DIR)/archs.o $(DIR)/cpu.o $(DIR)/gdt.o $(DIR)/idt.o $(DIR)/mmu.o $(DIR)/tss.o

$(LIB_$(DIR)): $(OBJ_$(DIR))
	$(AR) $(ARFLAGS) $@ $^

LIBS:=$(LIBS) $(LIB_$(DIR))
CLEAN:=$(CLEAN) $(LIB_$(DIR)) $(OBJ_$(DIR))
