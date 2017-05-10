CROSS_COMPILE=i386-unknown-elf-
AR=$(CROSS_COMPILE)ar
AS=$(CROSS_COMPILE)as
CC=$(CROSS_COMPILE)cc
LD=$(CROSS_COMPILE)ld
PP=$(CROSS_COMPILE)cc

%.i: %.c
	@echo PP $@
	@$(PP) -msoft-float -Wall -Werror -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2 -E -o $@ -I$(DIR_INCLUDE) -I$(DIR_SRC) -I$(DIR_LIB) $^

%.s: %.i
	@echo CC $@
	@$(CC) -msoft-float -Wall -Werror -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2 -c -S -o $@ $^

%.o: %.s
	@echo AS $@
	@$(AS) -c -o $@ $^
