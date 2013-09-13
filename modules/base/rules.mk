MOD:=modules/base/base.ko
OBJ:=modules/base/main.o modules/base/keyboard.o modules/base/mouse.o modules/base/terminal.o

$(MOD): $(OBJ) $(LIBFUDGE)
	$(LD) $(LDFLAGS) -o $@ $^

MODULES+=$(MOD)
MODULES_OBJECTS+=$(OBJ)
