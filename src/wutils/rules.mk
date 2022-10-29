B:=\
    $(DIR_SRC)/wutils/wcalc \

O:=\
    $(DIR_SRC)/wutils/wcalc.o \

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
    $(DIR_SRC)/wutils/wshell2 \

O:=\
    $(DIR_SRC)/wutils/wshell2.o \

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
