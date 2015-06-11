BIN_$(DIR):=$(DIR)/fudge
OBJ_$(DIR)_x86:=$(DIR)/x86/arch.o $(DIR)/x86/archs.o $(DIR)/x86/cpu.o $(DIR)/x86/gdt.o $(DIR)/x86/idt.o $(DIR)/x86/mmu.o $(DIR)/x86/tss.o
OBJ_$(DIR)_x86+=$(PACKAGES_PATH)/mboot/main.o $(PACKAGES_PATH)/mboot/mboot.o
OBJ_$(DIR)_arm:=$(DIR)/arm/arch.o $(DIR)/arm/cpu.o $(DIR)/arm/init.o $(DIR)/arm/kmi.o $(DIR)/arm/pic.o $(DIR)/arm/reg.o $(DIR)/arm/uart.o
OBJ_$(DIR)_arm+=$(PACKAGES_PATH)/std/arm/memcmp.o $(PACKAGES_PATH)/std/arm/memcpy.o $(PACKAGES_PATH)/std/arm/memmove.o $(PACKAGES_PATH)/std/arm/memset.o $(PACKAGES_PATH)/std/arm/setjmp.o $(PACKAGES_PATH)/std/arm/strcmp.o $(PACKAGES_PATH)/std/arm/strncmp.o $(PACKAGES_PATH)/std/arm/gcc/__aeabi_idiv.o $(PACKAGES_PATH)/std/arm/gcc/__aeabi_idivmod.o $(PACKAGES_PATH)/std/arm/gcc/__aeabi_uidiv.o $(PACKAGES_PATH)/std/arm/gcc/__aeabi_uidivmod.o $(PACKAGES_PATH)/std/arm/gcc/__clzsi2.o $(PACKAGES_PATH)/std/arm/gcc/__divsi3.o $(PACKAGES_PATH)/std/arm/gcc/__modsi3.o $(PACKAGES_PATH)/std/arm/gcc/__udivmodsi4.o $(PACKAGES_PATH)/std/arm/gcc/__udivsi3.o $(PACKAGES_PATH)/std/arm/gcc/__umodsi3.o
OBJ_$(DIR):=$(DIR)/binary.o $(DIR)/binary_elf.o $(DIR)/container.o $(DIR)/kernel.o $(DIR)/resource.o $(DIR)/scheduler.o $(DIR)/task.o $(DIR)/vfs.o $(DIR)/vfs_cpio.o $(DIR)/vfs_tar.o
OBJ_$(DIR)+=$(PACKAGES_PATH)/elf/elf.o $(PACKAGES_PATH)/tar/tar.o $(PACKAGES_PATH)/cpio/cpio.o $(PACKAGES_PATH)/fudge/ascii.o $(PACKAGES_PATH)/fudge/buffer.o $(PACKAGES_PATH)/fudge/memory.o $(PACKAGES_PATH)/fudge/list.o
OBJ_$(DIR)+=$(OBJ_$(DIR)_$(ARCH))

$(BIN_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $(LDFLAGS) -Tplatform/$(PLATFORM)/linker.ld $^

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(OBJ_$(DIR))
