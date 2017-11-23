CROSS_COMPILE:=i386-unknown-elf-

AR=$(CROSS_COMPILE)ar rcs
AS=$(CROSS_COMPILE)as -c -o
CC=$(CROSS_COMPILE)cc -Wall -Werror -msoft-float -ffreestanding -fno-asynchronous-unwind-tables -std=c89 -pedantic -O2 -S -o
NM=$(CROSS_COMPILE)nm -g
PP=$(CROSS_COMPILE)cc -Wall -Werror -nostdinc -std=c89 -pedantic -E -I$(DIR_INCLUDE) -I$(DIR_LIB) -I$(DIR_SRC) -o
DP=$(CROSS_COMPILE)cc -I$(DIR_INCLUDE) -I$(DIR_LIB) -I$(DIR_SRC) -MM -MT
LD_BIN=$(CROSS_COMPILE)ld -static -nostdlib -o
LD_KBIN=$(CROSS_COMPILE)ld -static -nostdlib -T$(DIR_LIB)/$(LOADER)/linker.ld -o
LD_KMOD=$(CROSS_COMPILE)ld -static -nostdlib -T$(DIR_SRC)/modules/linker.ld -r -o

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
