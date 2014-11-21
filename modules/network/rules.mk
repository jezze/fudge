MOD_$(DIR):=$(DIR)/network.ko.0
OBJ_$(DIR):=$(DIR)/main.o

$(MOD_$(DIR)): $(OBJ_$(DIR))
	$(LD) -o $@ $^ $(LDFLAGS)

MODULES:=$(MODULES) $(MOD_$(DIR))
CLEAN:=$(CLEAN) $(MOD_$(DIR)) $(OBJ_$(DIR))

PAR:=$(PAR).x
TMP_$(PAR):=$(CUR)
CUR:=$(DIR)

DIR:=$(CUR)/arp
include $(DIR)/rules.mk
DIR:=$(CUR)/ethernet
include $(DIR)/rules.mk
DIR:=$(CUR)/ipv4
include $(DIR)/rules.mk
DIR:=$(CUR)/ipv6
include $(DIR)/rules.mk

CUR:=$(TMP_$(PAR))
PAR:=$(basename $(PAR))
