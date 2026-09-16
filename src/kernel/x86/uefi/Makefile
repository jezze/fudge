BIN=uefi.efi
OBJ=uefi.o
CC=x86_64-w64-mingw32-gcc
CFLAGS=-c -ffreestanding -I/usr/include/efi -I/usr/include/efi/x86_64 -I/usr/include/protocol
LDFLAGS=-nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main

all: ${BIN}

clean:
	rm -f $(BIN) $(OBJ)

$(BIN): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $^
