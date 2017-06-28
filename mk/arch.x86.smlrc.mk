AR=i386-unknown-elf-ar rcs
AS=i386-unknown-elf-as -c -o
CC=smlrc -Wall -seg32 -nopp -no-leading-underscore -nobss
PP=smlrpp -Iinclude -Ilib -Isrc
LD_BIN=smlrl -elf -entry _start -o
LD_KBIN=i386-unknown-elf-ld -static -nostdlib -T$(DIR_SRC)/kernel/$(ARCH)/linker.ld -o
LD_KMOD=i386-unknown-elf-ld -static -nostdlib -T$(DIR_SRC)/modules/linker.ld -r -o
NASM=nasm -f elf -o

%.i: %.c
	@echo PP $@
	@$(PP) $^ > $@

%.asm: %.i
	@echo CC $@
	$(CC) $^ $@

%.o: %.asm
	@echo NASM $@
	@$(NASM) $@ $^

%.o: %.s
	@echo AS $@
	@$(AS) $@ $^
