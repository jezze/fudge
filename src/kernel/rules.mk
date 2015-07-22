BIN_$(DIR):=$(DIR)/fudge
OBJ_$(DIR):=$(DIR)/abi.o $(DIR)/binary.o $(DIR)/binary_elf.o $(DIR)/container.o $(DIR)/kernel.o $(DIR)/resource.o $(DIR)/task.o $(DIR)/vfs.o $(DIR)/vfs_cpio.o
OBJ_$(DIR)+=$(SRC_PATH)/format/elf.o $(SRC_PATH)/format/cpio.o $(SRC_PATH)/fudge/ascii.o $(SRC_PATH)/fudge/buffer.o $(SRC_PATH)/fudge/ctrl.o $(SRC_PATH)/fudge/memory.o $(SRC_PATH)/fudge/list.o

include $(DIR)/$(ARCH)/rules.mk

$(BIN_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) -Tplatform/$(PLATFORM)/linker.ld $^

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
