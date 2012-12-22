KERNEL_OBJECTS+=kernel/arm/arch.o
KERNEL_OBJECTS+=kernel/arm/cpu.o
KERNEL_OBJECTS+=kernel/arm/init.o
KERNEL_OBJECTS+=kernel/arm/kmi.o
KERNEL_OBJECTS+=kernel/arm/reg.o
KERNEL_OBJECTS+=kernel/arm/uart.o

$(KERNEL): $(KERNEL_OBJECTS) $(LIBKERNEL) $(LIBVERSATILEPB) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -Tlibs/versatilepb/linker.ld -o $@ $^
