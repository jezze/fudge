%.i: %.c
	@echo PP $@
	@$(TARGET)-cc -msoft-float -Wall -Werror -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2 -E -o $@ -I$(DIR_INCLUDE) -I$(DIR_SRC) -I$(DIR_LIB) $^

%.s: %.i
	@echo CC $@
	@$(TARGET)-cc -msoft-float -Wall -Werror -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2 -c -S -o $@ $^

%.o: %.s
	@echo AS $@
	@$(TARGET)-as -c -o $@ $^
