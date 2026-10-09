M:=\
    $(DIR_SRC)/modules/audio.ko \

N:=\
    $(DIR_SRC)/modules/audio.ko.map \

O:=\
    $(DIR_SRC)/modules/audio.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/base.ko \

N:=\
    $(DIR_SRC)/modules/base.ko.map \

O:=\
    $(DIR_SRC)/modules/bus.o \
    $(DIR_SRC)/modules/driver.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/block.ko \

N:=\
    $(DIR_SRC)/modules/block.ko.map \

O:=\
    $(DIR_SRC)/modules/block.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/clock.ko \

N:=\
    $(DIR_SRC)/modules/clock.ko.map \

O:=\
    $(DIR_SRC)/modules/clock.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/console.ko \

N:=\
    $(DIR_SRC)/modules/console.ko.map \

O:=\
    $(DIR_SRC)/modules/console.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/ethernet.ko \

N:=\
    $(DIR_SRC)/modules/ethernet.ko.map \

O:=\
    $(DIR_SRC)/modules/ethernet.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/net/net.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/info.ko \

N:=\
    $(DIR_SRC)/modules/info.ko.map \

O:=\
    $(DIR_SRC)/modules/info.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/keyboard.ko \

N:=\
    $(DIR_SRC)/modules/keyboard.ko.map \

O:=\
    $(DIR_SRC)/modules/keyboard.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/log.ko \

N:=\
    $(DIR_SRC)/modules/log.ko.map \

O:=\
    $(DIR_SRC)/modules/log.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/mouse.ko \

N:=\
    $(DIR_SRC)/modules/mouse.ko.map \

O:=\
    $(DIR_SRC)/modules/mouse.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/timer.ko \

N:=\
    $(DIR_SRC)/modules/timer.ko.map \

O:=\
    $(DIR_SRC)/modules/timer.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk

M:=\
    $(DIR_SRC)/modules/video.ko \

N:=\
    $(DIR_SRC)/modules/video.ko.map \

O:=\
    $(DIR_SRC)/modules/video.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
include $(DIR_SRC)/modules/arch/$(ARCH)/rules.mk
