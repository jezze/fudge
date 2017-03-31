$(DIR_SRC)/%.i: $(DIR_SRC)/%.c
	@echo PP $@
	@$(TARGET)-cc -msoft-float -Wall -Werror -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2 -E -o $@ -I$(DIR_INCLUDE) -I$(DIR_SRC) $^

$(DIR_SRC)/%.s: $(DIR_SRC)/%.i
	@echo CC $@
	@$(TARGET)-cc -msoft-float -Wall -Werror -ffreestanding -nostdlib -nostdinc -std=c89 -pedantic -O2 -c -S -o $@ $^

$(DIR_SRC)/%.o: $(DIR_SRC)/%.s
	@echo AS $@
	@$(TARGET)-as -c -o $@ $^
