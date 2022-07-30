AR=$(TARGET)-ar rcs
AS=$(TARGET)-as -c -o
CC=$(TARGET)-gcc -Wall -Werror -Wno-overlength-strings -msoft-float -ffreestanding -fno-asynchronous-unwind-tables -std=c89 -pedantic -O2 -S -o
NM=$(TARGET)-nm -gp
PP=$(TARGET)-gcc -Wall -Werror -nostdinc -std=c89 -pedantic -E -I$(DIR_INCLUDE) -I$(DIR_LIB) -I$(DIR_SRC) -o
LD_BIN=$(TARGET)-ld -s -static -nostdlib -o
LD_KBIN=$(TARGET)-ld -static -nostdlib -T$(DIR_SRC)/kernel/$(ARCH)/$(LOADER)/linker.ld -o
LD_KMOD=$(TARGET)-ld -static -nostdlib -T$(DIR_SRC)/modules/linker.ld -r -o

%.i: %.c
	@echo PP $@
	@$(PP) $@ $^

%.s: %.i
	@echo CC $@
	@$(CC) $@ $^

%.o: %.s
	@echo AS $@
	@$(AS) $@ $^

%.map: %
	@echo NM $@
	@$(NM) $^ > $@

