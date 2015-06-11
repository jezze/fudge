BIN_$(DIR):=$(DIR)/fudge
OBJ_$(DIR)_x86:=$(DIR)/x86/arch.o $(DIR)/x86/archs.o $(DIR)/x86/cpu.o $(DIR)/x86/gdt.o $(DIR)/x86/idt.o $(DIR)/x86/mmu.o $(DIR)/x86/tss.o
OBJ_$(DIR)_x86+=$(SRC_PATH)/mboot/main.o $(SRC_PATH)/mboot/mboot.o
OBJ_$(DIR)_arm:=$(DIR)/arm/arch.o $(DIR)/arm/cpu.o $(DIR)/arm/init.o $(DIR)/arm/kmi.o $(DIR)/arm/pic.o $(DIR)/arm/reg.o $(DIR)/arm/uart.o
OBJ_$(DIR)_arm+=$(SRC_PATH)/std/arm/memcmp.o $(SRC_PATH)/std/arm/memcpy.o $(SRC_PATH)/std/arm/memmove.o $(SRC_PATH)/std/arm/memset.o $(SRC_PATH)/std/arm/setjmp.o $(SRC_PATH)/std/arm/strcmp.o $(SRC_PATH)/std/arm/strncmp.o $(SRC_PATH)/std/arm/gcc/__aeabi_idiv.o $(SRC_PATH)/std/arm/gcc/__aeabi_idivmod.o $(SRC_PATH)/std/arm/gcc/__aeabi_uidiv.o $(SRC_PATH)/std/arm/gcc/__aeabi_uidivmod.o $(SRC_PATH)/std/arm/gcc/__clzsi2.o $(SRC_PATH)/std/arm/gcc/__divsi3.o $(SRC_PATH)/std/arm/gcc/__modsi3.o $(SRC_PATH)/std/arm/gcc/__udivmodsi4.o $(SRC_PATH)/std/arm/gcc/__udivsi3.o $(SRC_PATH)/std/arm/gcc/__umodsi3.o
OBJ_$(DIR):=$(DIR)/binary.o $(DIR)/binary_elf.o $(DIR)/container.o $(DIR)/kernel.o $(DIR)/resource.o $(DIR)/scheduler.o $(DIR)/task.o $(DIR)/vfs.o $(DIR)/vfs_cpio.o $(DIR)/vfs_tar.o
OBJ_$(DIR)+=$(SRC_PATH)/elf/elf.o $(SRC_PATH)/tar/tar.o $(SRC_PATH)/cpio/cpio.o $(SRC_PATH)/fudge/ascii.o $(SRC_PATH)/fudge/buffer.o $(SRC_PATH)/fudge/memory.o $(SRC_PATH)/fudge/list.o
OBJ_$(DIR)+=$(OBJ_$(DIR)_$(ARCH))

$(BIN_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) -Tplatform/$(PLATFORM)/linker.ld $^

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
