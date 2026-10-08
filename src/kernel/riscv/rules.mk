O:=\
    $(O) \
    $(DIR_SRC)/kernel/riscv/debug.o \
    $(DIR_SRC)/kernel/riscv/arch.o \

include $(DIR_SRC)/kernel/riscv/$(LOADER)/rules.mk
