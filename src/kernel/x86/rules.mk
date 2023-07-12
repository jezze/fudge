O:=\
    $(O) \
    $(DIR_SRC)/kernel/x86/arch.o \
    $(DIR_SRC)/kernel/x86/cpu.o \
    $(DIR_SRC)/kernel/x86/gdt.o \
    $(DIR_SRC)/kernel/x86/idt.o \
    $(DIR_SRC)/kernel/x86/io.o \
    $(DIR_SRC)/kernel/x86/isr.o \
    $(DIR_SRC)/kernel/x86/mmu.o \
    $(DIR_SRC)/kernel/x86/pic.o \
    $(DIR_SRC)/kernel/x86/tss.o \

include $(DIR_SRC)/kernel/x86/$(LOADER)/rules.mk
