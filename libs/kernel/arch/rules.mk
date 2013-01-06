

kernel/%.o: kernel/%.c
	$(CC) -c $(CFLAGS) -Ilibs -o $@ $<
