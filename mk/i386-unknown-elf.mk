AR=$(TARGET)-ar
AS=$(TARGET)-as
CC=$(TARGET)-cc
LD=$(TARGET)-ld
NM=$(TARGET)-nm
PP=$(TARGET)-cc
DP=$(TARGET)-cc
AR_FLAGS=rcs
AS_FLAGS=-c
CC_FLAGS=-Wall -Werror -Wno-overlength-strings -msoft-float -ffreestanding -fno-asynchronous-unwind-tables -std=c89 -pedantic -O2 -S
NM_FLAGS=-gp
PP_FLAGS=-Wall -Werror -nostdinc -std=c89 -pedantic -E -I$(DIR_INCLUDE) -I$(DIR_LIB) -I$(DIR_SRC)
DP_FLAGS=-I$(DIR_INCLUDE) -I$(DIR_LIB) -I$(DIR_SRC) -MM -MT
LD_FLAGS_BIN=-s -static -nostdlib
LD_FLAGS_KBIN=-static -nostdlib -T$(DIR_SRC)/kernel/$(ARCH)/$(LOADER)/linker.ld
LD_FLAGS_KMOD=-static -nostdlib -T$(DIR_SRC)/modules/linker.ld -r

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
	@$(NM) $(NM_FLAGS) $^ > $@

