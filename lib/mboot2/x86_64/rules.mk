O:=\
    $(O) \
    $(DIR_LIB)/mboot2/x86_64/cpio.o \
    $(DIR_LIB)/mboot2/x86_64/elf.o \
    $(DIR_LIB)/mboot2/x86_64/init.o \
    $(DIR_LIB)/mboot2/x86_64/mboot.o \

LD_SCRIPT_KBIN:=$(DIR_LIB)/mboot2/x86_64/linker.ld
