O:=\
    $(O) \
    $(DIR_SRC)/kernel/x86_64/debug.o \
    $(DIR_SRC)/kernel/x86_64/arch.o \
    $(DIR_SRC)/kernel/x86_64/cpu.o \
    $(DIR_SRC)/kernel/x86_64/gdt.o \
    $(DIR_SRC)/kernel/x86_64/idt.o \
    $(DIR_SRC)/kernel/x86_64/io.o \
    $(DIR_SRC)/kernel/x86_64/isr.o \
    $(DIR_SRC)/kernel/x86_64/mmu.o \
    $(DIR_SRC)/kernel/x86_64/tss.o \
    $(DIR_SRC)/kernel/x86/pic.o \
    $(DIR_SRC)/kernel/x86/udebug.o \

include $(DIR_SRC)/kernel/x86_64/$(LOADER)/rules.mk
