L:=\
    $(DIR_LIB)/fudge/fudge.a \

O:=\
    $(DIR_LIB)/fudge/ascii.o \
    $(DIR_LIB)/fudge/box.o \
    $(DIR_LIB)/fudge/cpio.o \
    $(DIR_LIB)/fudge/ctrl.o \
    $(DIR_LIB)/fudge/elf.o \
    $(DIR_LIB)/fudge/event.o \
    $(DIR_LIB)/fudge/keymap.o \
    $(DIR_LIB)/fudge/list.o \
    $(DIR_LIB)/fudge/md5.o \
    $(DIR_LIB)/fudge/memory.o \
    $(DIR_LIB)/fudge/pcf.o \
    $(DIR_LIB)/fudge/regexp.o \
    $(DIR_LIB)/fudge/ring.o \
    $(DIR_LIB)/fudge/sha1.o \
    $(DIR_LIB)/fudge/spinlock.o \
    $(DIR_LIB)/fudge/tar.o \
    $(DIR_LIB)/fudge/utf8.o \

include $(DIR_LIB)/fudge/$(ARCH)/rules.mk
include $(DIR_MK)/lib.mk
