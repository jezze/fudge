#include modules/block/ext2/rules.mk

MOD:=modules/block/block.ko
OBJ:=modules/block/main.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
