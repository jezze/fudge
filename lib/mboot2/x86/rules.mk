O:=\
    $(O) \
    $(DIR_LIB)/mboot2/x86/cpio.o \
    $(DIR_LIB)/mboot2/x86/elf.o \
    $(DIR_LIB)/mboot2/x86/init.o \
    $(DIR_LIB)/mboot2/x86/mboot.o \

LD_SCRIPT_KBIN:=$(DIR_LIB)/mboot2/x86/linker.ld
