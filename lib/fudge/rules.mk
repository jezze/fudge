L:=\
    $(DIR_LIB)/fudge/fudge.a \

O:=\
    $(DIR_LIB)/fudge/base64.o \
    $(DIR_LIB)/fudge/buffer.o \
    $(DIR_LIB)/fudge/cpio.o \
    $(DIR_LIB)/fudge/cstring.o \
    $(DIR_LIB)/fudge/elf.o \
    $(DIR_LIB)/fudge/keys.o \
    $(DIR_LIB)/fudge/list.o \
    $(DIR_LIB)/fudge/message.o \
    $(DIR_LIB)/fudge/mtwist.o \
    $(DIR_LIB)/fudge/p9p.o \
    $(DIR_LIB)/fudge/pcf.o \
    $(DIR_LIB)/fudge/regexp.o \
    $(DIR_LIB)/fudge/ring.o \
    $(DIR_LIB)/fudge/spinlock.o \
    $(DIR_LIB)/fudge/tar.o \
    $(DIR_LIB)/fudge/time.o \
    $(DIR_LIB)/fudge/url.o \
    $(DIR_LIB)/fudge/utf8.o \

include $(DIR_LIB)/fudge/$(ARCH)/rules.mk
include $(DIR_MK)/lib.mk
