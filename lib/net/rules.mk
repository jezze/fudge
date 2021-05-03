L:=\
    $(DIR_LIB)/net/net.a \

O:=\
    $(DIR_LIB)/net/arp.o \
    $(DIR_LIB)/net/dns.o \
    $(DIR_LIB)/net/ethernet.o \
    $(DIR_LIB)/net/icmp.o \
    $(DIR_LIB)/net/ipv4.o \
    $(DIR_LIB)/net/net.o \
    $(DIR_LIB)/net/tcp.o \
    $(DIR_LIB)/net/udp.o \

include $(DIR_MK)/lib.mk
