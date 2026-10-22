AR=$(TARGET)-ar
AS=$(TARGET)-as
CC=$(TARGET)-gcc
DP=$(TARGET)-gcc
LD=$(TARGET)-ld
NM=$(TARGET)-nm
OC=$(TARGET)-objcopy
PP=$(TARGET)-gcc
AR_FLAGS=rcs
AS_FLAGS=-c -m68040
CC_FLAGS_FLOAT=-msoft-float
CC_FLAGS=-Wall -Werror -Wno-overlength-strings -m68040 -msoft-float -fno-pic -fno-pie -ffreestanding -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-stack-protector -std=c89 -pedantic -O2 -S
DP_FLAGS=-DARCH_AMIGA -I$(DIR_INCLUDE) -I$(DIR_LIB) -I$(DIR_SRC) -I$(DIR_SRC)/kernel/$(ARCH) -MM -MT
LD_FLAGS_BIN=-z max-page-size=0x1000 -Ttext-segment=0x08048000 -s -static -nostdlib
LD_FLAGS_KBIN=-z max-page-size=0x1000 -static -nostdlib -T$(DIR_MK)/m68k-amiga.ld
LD_FLAGS_KMOD=-T$(DIR_MK)/kmod.ld -r
NM_FLAGS=-gp
PP_FLAGS=-DARCH_AMIGA -Wall -Werror -nostdinc -std=c89 -pedantic -E -I$(DIR_INCLUDE) -I$(DIR_LIB) -I$(DIR_SRC) -I$(DIR_SRC)/kernel/$(ARCH)

%.d: %.s
	@echo DP $@
	@echo $*.o: > $*.d

%.d: %.c
	@echo DP $@
	@$(DP) $(DP_FLAGS) $*.o $*.c > $*.d

%.i: %.c
	@echo PP $@
	@$(PP) $(PP_FLAGS) -o $@ $^

%.s: %.i
	@echo CC $@
	@$(CC) $(CC_FLAGS) -o $@ $^

%.o: %.s
	@echo AS $@
	@$(AS) $(AS_FLAGS) -o $@ $^

%.map: %
	@echo NM $@
	@$(NM) $(NM_FLAGS) $^ | sort > $@

