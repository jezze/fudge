O:=\
    $(O) \
    $(DIR_LIB)/mboot2/cpio.o \

LD_SCRIPT_KBIN:=$(DIR_LIB)/mboot2/linker.ld

include $(DIR_LIB)/mboot2/$(ARCH)/rules.mk
