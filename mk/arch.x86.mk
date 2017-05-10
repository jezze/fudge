AR=i386-unknown-elf-ar rcs
AS=i386-unknown-elf-as -c -o
CC=i386-unknown-elf-cc -msoft-float -Wall -Werror -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2 -c -S -o
LD=i386-unknown-elf-ld -static -o
PP=i386-unknown-elf-cc -msoft-float -Wall -Werror -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2 -E -I$(DIR_INCLUDE) -I$(DIR_SRC) -I$(DIR_LIB) -o

%.i: %.c
	@echo PP $@
	@$(PP) $@ $^

%.s: %.i
	@echo CC $@
	@$(CC) $@ $^

%.o: %.s
	@echo AS $@
	@$(AS) $@ $^
