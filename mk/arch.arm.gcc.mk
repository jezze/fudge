AR=arm-unknown-eabi-ar rcs
AS=arm-unknown-eabi-as -c -o
CC=arm-unknown-eabi-cc -msoft-float -Wall -Werror -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2 -c -S -o
NM=arm-unknown-eabi-nm -g
LD=arm-unknown-eabi-ld -static -o
PP=arm-unknown-eabi-cc -msoft-float -Wall -Werror -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2 -E -I$(DIR_INCLUDE) -I$(DIR_SRC) -I$(DIR_LIB) -o

%.i: %.c
	@echo PP $@
	@$(PP) $@ $^

%.s: %.i
	@echo CC $@
	@$(CC) $@ $^

%.o: %.s
	@echo AS $@
	@$(AS) $@ $^
