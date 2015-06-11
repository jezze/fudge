BIN_$(DIR):=$(DIR)/fudge

KERNEL_DEPS_x86:=$(PACKAGES_PATH)/arch/x86/arch.o $(PACKAGES_PATH)/arch/x86/archs.o $(PACKAGES_PATH)/arch/x86/cpu.o $(PACKAGES_PATH)/arch/x86/gdt.o $(PACKAGES_PATH)/arch/x86/idt.o $(PACKAGES_PATH)/arch/x86/mmu.o $(PACKAGES_PATH)/arch/x86/tss.o
KERNEL_DEPS_x86+=$(PACKAGES_PATH)/mboot/main.o $(PACKAGES_PATH)/mboot/mboot.o
KERNEL_DEPS_arm:=$(PACKAGES_PATH)/arch/arm/arch.o $(PACKAGES_PATH)/arch/arm/cpu.o $(PACKAGES_PATH)/arch/arm/init.o $(PACKAGES_PATH)/arch/arm/kmi.o $(PACKAGES_PATH)/arch/arm/pic.o $(PACKAGES_PATH)/arch/arm/reg.o $(PACKAGES_PATH)/arch/arm/uart.o
KERNEL_DEPS_arm+=$(PACKAGES_PATH)/std/arm/memcmp.o $(PACKAGES_PATH)/std/arm/memcpy.o $(PACKAGES_PATH)/std/arm/memmove.o $(PACKAGES_PATH)/std/arm/memset.o $(PACKAGES_PATH)/std/arm/setjmp.o $(PACKAGES_PATH)/std/arm/strcmp.o $(PACKAGES_PATH)/std/arm/strncmp.o $(PACKAGES_PATH)/std/arm/gcc/__aeabi_idiv.o $(PACKAGES_PATH)/std/arm/gcc/__aeabi_idivmod.o $(PACKAGES_PATH)/std/arm/gcc/__aeabi_uidiv.o $(PACKAGES_PATH)/std/arm/gcc/__aeabi_uidivmod.o $(PACKAGES_PATH)/std/arm/gcc/__clzsi2.o $(PACKAGES_PATH)/std/arm/gcc/__divsi3.o $(PACKAGES_PATH)/std/arm/gcc/__modsi3.o $(PACKAGES_PATH)/std/arm/gcc/__udivmodsi4.o $(PACKAGES_PATH)/std/arm/gcc/__udivsi3.o $(PACKAGES_PATH)/std/arm/gcc/__umodsi3.o
KERNEL_DEPS:=$(PACKAGES_PATH)/kernel/binary.o $(PACKAGES_PATH)/kernel/binary_elf.o $(PACKAGES_PATH)/kernel/container.o $(PACKAGES_PATH)/kernel/kernel.o $(PACKAGES_PATH)/kernel/resource.o $(PACKAGES_PATH)/kernel/scheduler.o $(PACKAGES_PATH)/kernel/task.o $(PACKAGES_PATH)/kernel/vfs.o $(PACKAGES_PATH)/kernel/vfs_cpio.o $(PACKAGES_PATH)/kernel/vfs_tar.o
KERNEL_DEPS+=$(PACKAGES_PATH)/elf/elf.o $(PACKAGES_PATH)/tar/tar.o $(PACKAGES_PATH)/cpio/cpio.o
KERNEL_DEPS+=$(PACKAGES_PATH)/fudge/ascii.o $(PACKAGES_PATH)/fudge/buffer.o $(PACKAGES_PATH)/fudge/memory.o $(PACKAGES_PATH)/fudge/list.o
KERNEL_DEPS+=$(KERNEL_DEPS_$(ARCH))

$(BIN_$(DIR)): $(KERNEL_DEPS)
	$(LD) -o $@ $(LDFLAGS) -Tplatform/$(PLATFORM)/linker.ld $^

BINS:=$(BINS) $(BIN_$(DIR))
CLEAN:=$(CLEAN) $(BIN_$(DIR)) $(KERNEL_DEPS)
