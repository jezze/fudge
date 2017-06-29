AR=i386-unknown-elf-ar rcs
AS=i386-unknown-elf-as -c -o
CC=i386-unknown-elf-cc -Wall -Werror -msoft-float -ffreestanding -fno-asynchronous-unwind-tables -std=c89 -pedantic -O2 -S -o
NM=i386-unknown-elf-nm -g
PP=i386-unknown-elf-cc -Wall -Werror -nostdinc -std=c89 -pedantic -E -I$(DIR_INCLUDE) -I$(DIR_SRC) -I$(DIR_LIB) -o
LD_BIN=i386-unknown-elf-ld -static -nostdlib -o
LD_KBIN=i386-unknown-elf-ld -static -nostdlib -T$(DIR_LIB)/$(LOADER)/linker.ld -o
LD_KMOD=i386-unknown-elf-ld -static -nostdlib -T$(DIR_SRC)/modules/linker.ld -r -o

%.i: %.c
	@echo PP $@
	@$(PP) $@ $^

%.s: %.i
	@echo CC $@
	@$(CC) $@ $^

%.o: %.s
	@echo AS $@
	@$(AS) $@ $^
