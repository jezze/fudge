O:=\
    $(O) \
    $(DIR_SRC)/kernel/arm/arch.o \
    $(DIR_SRC)/kernel/arm/cpu.o \
    $(DIR_SRC)/kernel/arm/kmi.o \
    $(DIR_SRC)/kernel/arm/pic.o \
    $(DIR_SRC)/kernel/arm/reg.o \
    $(DIR_SRC)/kernel/arm/uart.o

include $(DIR_SRC)/kernel/arm/$(LOADER)/rules.mk
