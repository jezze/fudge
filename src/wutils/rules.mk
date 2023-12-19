B:=\
    $(DIR_SRC)/wutils/wabout \

O:=\
    $(DIR_SRC)/wutils/wabout.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/wutils/wcalc \

O:=\
    $(DIR_SRC)/wutils/kv.o \
    $(DIR_SRC)/wutils/wcalc.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/wutils/wfile \

O:=\
    $(DIR_SRC)/wutils/kv.o \
    $(DIR_SRC)/wutils/wfile.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/wutils/wrun \

O:=\
    $(DIR_SRC)/wutils/wrun.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \
    $(DIR_LIB)/socket/socket.a \
    $(DIR_LIB)/net/net.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/wutils/wshell \

O:=\
    $(DIR_SRC)/wutils/wshell.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/wutils/wsettings \

O:=\
    $(DIR_SRC)/wutils/wsettings.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk

B:=\
    $(DIR_SRC)/wutils/wtest \

O:=\
    $(DIR_SRC)/wutils/wtest.o \

L:=\
    $(DIR_LIB)/abi/abi.a \
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/bin.mk
