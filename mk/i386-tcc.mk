AR=$(TARGET)
AS=$(TARGET)
CC=$(TARGET)
LD=$(TARGET)
NM=touch
AR_FLAGS=-r
AS_FLAGS=-c
CC_FLAGS=-c -Wall -Werror -Wno-overlength-strings -msoft-float -ffreestanding -fno-asynchronous-unwind-tables -std=c89 -pedantic -O2 -nostdlib -I$(DIR_INCLUDE) -I$(DIR_LIB) -I$(DIR_SRC)
LD_FLAGS=-s -static -nostdlib
LD_FLAGS_KBIN=-static -nostdlib -g -Wl,-Ttext=0x100000
LD_FLAGS_KMOD=-static -nostdlib -r

%.o: %.c
	@echo CC $@
	@$(CC) $(CC_FLAGS) -o $@ $^

%.o: %.s
	@echo AS $@
	@$(AS) $(AS_FLAGS) -o $@ $^

%.map: %
	@echo NM $@
	@$(NM) $@

