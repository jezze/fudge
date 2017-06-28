AR=i386-unknown-elf-ar rcs
AS=i386-unknown-elf-as -c -o
CC=i386-unknown-elf-cc -Wall -Werror -msoft-float -ffreestanding -fno-asynchronous-unwind-tables -std=c89 -pedantic -O2 -S -o
LD=i386-unknown-elf-ld -static -nostdlib -o
PP=i386-unknown-elf-cc -Wall -Werror -nostdinc -std=c89 -pedantic -E -I$(DIR_INCLUDE) -I$(DIR_SRC) -I$(DIR_LIB) -o

%.i: %.c
	@echo PP $@
	@$(PP) $@ $^

%.s: %.i
	@echo CC $@
	@$(CC) $@ $^

%.o: %.s
	@echo AS $@
	@$(AS) $@ $^
