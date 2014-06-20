LIB_$(DIR):=$(DIR)/libkernel.a
OBJ_$(DIR):=$(DIR)/binary.o $(DIR)/binary_elf.o $(DIR)/container.o $(DIR)/error.o $(DIR)/kernel.o $(DIR)/rendezvous.o $(DIR)/resource.o $(DIR)/scheduler.o $(DIR)/task.o $(DIR)/vfs.o $(DIR)/vfs_cpio.o $(DIR)/vfs_tar.o

$(LIB_$(DIR)): $(OBJ_$(DIR))
	$(AR) $(ARFLAGS) $@ $^

LIBS_BUILD:=$(LIBS_BUILD) $(LIB_$(DIR))
LIBS_CLEAN:=$(LIBS_CLEAN) $(LIB_$(DIR)) $(OBJ_$(DIR))
