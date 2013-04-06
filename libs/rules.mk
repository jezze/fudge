include libs/fudge/rules.mk
include libs/posix/rules.mk
include libs/kernel/rules.mk
include libs/mboot/rules.mk

libs/%.o: libs/%.s
	$(AS) $(ASFLAGS) -o $@ $<

libs/%.o: libs/%.c
	$(CC) -c $(CFLAGS) -Ilibs -o $@ $<
