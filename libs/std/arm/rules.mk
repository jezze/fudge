LIB_$(DIR):=$(BUILD_LIB)/libstd.a
OBJ_$(DIR):=$(DIR)/memcmp.o $(DIR)/memcpy.o $(DIR)/memmove.o $(DIR)/memset.o $(DIR)/setjmp.o $(DIR)/strcmp.o $(DIR)/strncmp.o $(DIR)/gcc/__aeabi_idiv.o $(DIR)/gcc/__aeabi_idivmod.o $(DIR)/gcc/__aeabi_uidiv.o $(DIR)/gcc/__aeabi_uidivmod.o $(DIR)/gcc/__clzsi2.o $(DIR)/gcc/__divsi3.o $(DIR)/gcc/__modsi3.o $(DIR)/gcc/__udivmodsi4.o $(DIR)/gcc/__udivsi3.o $(DIR)/gcc/__umodsi3.o

$(LIB_$(DIR)): $(OBJ_$(DIR))
	$(AR) $(ARFLAGS) $@ $^

LIBS:=$(LIBS) $(LIB_$(DIR))
CLEAN:=$(CLEAN) $(LIB_$(DIR)) $(OBJ_$(DIR))
