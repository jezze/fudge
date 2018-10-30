CROSS_COMPILE:=i386-unknown-elf-

AR=$(CROSS_COMPILE)ar rcs
AS=$(CROSS_COMPILE)as -c -o
CC=smlrcc -S -no-leading-underscore
NM=$(CROSS_COMPILE)nm -g
PP=smlrpp -I$(DIR_INCLUDE) -I$(DIR_LIB) -I$(DIR_SRC)
LD_BIN=smlrl -elf -entry _start -o
LD_KBIN=$(CROSS_COMPILE)ld -static -nostdlib -T$(DIR_LIB)/$(LOADER)/linker.ld -o
LD_KMOD=$(CROSS_COMPILE)ld -static -nostdlib -T$(DIR_SRC)/modules/linker.ld -r -o
NASM=nasm -f elf -o

%.d: %.s
	@echo DP $@
	@echo $*.o: > $*.d

%.d: %.c
	@echo DP $@
	@echo $*.o: > $*.d

%.i: %.c
	@echo PP $@
	@$(PP) $^ > $@

%.asm: %.i
	@echo CC $@
	@$(CC) $^

%.o: %.s
	@echo AS $@
	@$(AS) $@ $^

%.o: %.asm
	@echo NASM $@
	@$(NASM) $@ $^
