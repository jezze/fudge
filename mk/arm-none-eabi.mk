AR=$(TARGET)-ar
AS=$(TARGET)-as
CC=$(TARGET)-gcc
DP=$(TARGET)-gcc
LD=$(TARGET)-ld
NM=$(TARGET)-nm
PP=$(TARGET)-gcc
AR_FLAGS=rcs
AS_FLAGS=-c -mcpu=arm926ej-s
CC_FLAGS_FLOAT=-msoft-float
CC_FLAGS=-Wall -Werror -Wno-overlength-strings -marm -march=armv5t -mtune=arm926ej-s -msoft-float -ffreestanding -fno-asynchronous-unwind-tables -std=c89 -pedantic -O2 -S
DP_FLAGS=-I$(DIR_INCLUDE) -I$(DIR_LIB) -I$(DIR_SRC) -I$(DIR_SRC)/kernel/$(ARCH) -MM -MT
LD_FLAGS_BIN=-z max-page-size=0x1000 -Ttext-segment=0x08048000 -s -static -nostdlib
LD_FLAGS_KBIN=-z max-page-size=0x1000 -static -nostdlib -T$(LD_SCRIPT_KBIN)
LD_FLAGS_KMOD=-T$(DIR_SRC)/modules/linker.ld -r
NM_FLAGS=-gp
PP_FLAGS=-Wall -Werror -nostdinc -std=c89 -pedantic -E -I$(DIR_INCLUDE) -I$(DIR_LIB) -I$(DIR_SRC) -I$(DIR_SRC)/kernel/$(ARCH)

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
