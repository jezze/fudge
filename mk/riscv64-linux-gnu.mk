AR=$(TARGET)-ar rcs
AS=$(TARGET)-as -c -o
CC=$(TARGET)-gcc -Wall -Werror -ffreestanding -fno-asynchronous-unwind-tables -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -std=c89 -pedantic -O2 -S -o
NM=$(TARGET)-nm -gp
PP=$(TARGET)-gcc -Wall -Werror -nostdinc -std=c89 -pedantic -E -I$(DIR_INCLUDE) -I$(DIR_LIB) -I$(DIR_SRC) -o
DP=$(TARGET)-gcc -I$(DIR_INCLUDE) -I$(DIR_LIB) -I$(DIR_SRC) -MM -MT
LD_BIN=$(TARGET)-ld -s -static -nostdlib -o
LD_KBIN=$(TARGET)-ld -static -nostdlib -T$(DIR_LIB)/$(LOADER)/$(ARCH)/linker.ld -o
LD_KMOD=$(TARGET)-ld -static -nostdlib -T$(DIR_SRC)/modules/linker.ld -r -o

%.d: %.s
	@echo DP $@
	@echo $*.o: > $*.d

%.d: %.c
	@echo DP $@
	@$(DP) $*.o $*.c > $*.d

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

