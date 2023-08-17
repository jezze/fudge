AR=$(TARGET)
AS=$(TARGET)
CC=$(TARGET)
DP=$(TARGET)
LD=$(TARGET)
NM=touch
PP=$(TARGET)
AR_FLAGS=-r
AS_FLAGS=-c
CC_FLAGS=-c -Wall -Werror -Wno-overlength-strings -msoft-float -ffreestanding -fno-asynchronous-unwind-tables -std=c89 -pedantic -O2 -nostdlib -I$(DIR_INCLUDE) -I$(DIR_LIB) -I$(DIR_SRC)
DP_FLAGS= -MM -MT
LD_FLAGS=-s -static -nostdlib
LD_FLAGS_KBIN=-static -nostdlib -Wl,-Ttext=0x100000
LD_FLAGS_KMOD=-static -nostdlib -r
PP_FLAGS=-Wall -Werror -nostdinc -std=c89 -pedantic -E -I$(DIR_INCLUDE) -I$(DIR_LIB) -I$(DIR_SRC)

%.d: %.s
	@echo DP $@
	@echo $*.o: > $*.d

%.d: %.c
	@echo DP $@
	@$(DP) $(DP_FLAGS) $*.o $*.c > $*.d

%.o: %.c
	@echo CC $@
	@$(CC) $(CC_FLAGS) -o $@ $^

%.o: %.s
	@echo AS $@
	@$(AS) $(AS_FLAGS) -o $@ $^

%.map: %
	@echo NM $@
	@$(NM) $@

