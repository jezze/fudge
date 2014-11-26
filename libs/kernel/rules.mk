LIB_$(DIR):=$(BUILD_LIB)/libkernel.a
OBJ_$(DIR):=$(DIR)/binary.o $(DIR)/binary_elf.o $(DIR)/container.o $(DIR)/error.o $(DIR)/kernel.o $(DIR)/resource.o $(DIR)/scheduler.o $(DIR)/task.o $(DIR)/vfs.o $(DIR)/vfs_cpio.o $(DIR)/vfs_tar.o

$(LIB_$(DIR)): $(OBJ_$(DIR))
	$(AR) $(ARFLAGS) $@ $^

LIBS:=$(LIBS) $(LIB_$(DIR))
CLEAN:=$(CLEAN) $(LIB_$(DIR)) $(OBJ_$(DIR))
