KERNEL=kernel/fudge

include kernel/$(ARCH)/rules.mk

kernel/%.o: kernel/%.c
	$(CC) -c $(CFLAGS) -Ilibs -o $@ $<
