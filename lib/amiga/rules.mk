L:=\
    $(DIR_LIB)/amiga/amiga.a \

O:=\
    $(DIR_LIB)/amiga/amiga.o \
    $(DIR_LIB)/amiga/cpio.o \
    $(DIR_LIB)/amiga/elf.o \
    $(DIR_LIB)/amiga/init.o \

include $(DIR_MK)/lib.mk

all: fs-uae/hd/$(KERNEL)boot

fs-uae/hd/$(KERNEL)boot: $(DIR_LIB)/amiga/hunk.s $(KERNEL) $(RAMDISK)
	@echo HUNK $@
	@$(OC) -O binary $(KERNEL) $(KERNEL).bin
	@$(AS) $(AS_FLAGS) --defsym KERNEL=0x$$($(NM) $(KERNEL) | sed -n 's/ T _start$$//p') -o $(DIR_LIB)/amiga/hunk.o $(DIR_LIB)/amiga/hunk.s
	@$(OC) -O binary $(DIR_LIB)/amiga/hunk.o $@
	@rm -f $(KERNEL).bin

OBJ:=$(OBJ) $(KERNEL)boot $(DIR_LIB)/amiga/hunk.o
