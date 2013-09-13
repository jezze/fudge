include modules/net/arp/rules.mk
include modules/net/ipv4/rules.mk
include modules/net/ipv6/rules.mk

MOD:=modules/net/net.ko
OBJ:=modules/net/main.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
