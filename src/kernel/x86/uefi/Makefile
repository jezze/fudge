BIN=uefi.efi
OBJ=uefi.o
CC=x86_64-w64-mingw32-gcc
CFLAGS=-c -ffreestanding -I/usr/include/efi -I/usr/include/efi/x86_64 -I/usr/include/protocol 
LDFLAGS=-nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main
FIRMWARE=ovmf.fd
#FIRMWARE=/usr/share/ovmf/x64/OVMF.4m.fd

all: uefi.efi image/EFI/BOOT/BOOTX64.EFI

clean:
	rm -f $(BIN) $(OBJ)
	rm -rf image

$(BIN): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $^

image:
	mkdir -p $@

image/EFI: image
	mkdir -p $@

image/EFI/BOOT: image/EFI
	mkdir -p $@

image/EFI/BOOT/BOOTX64.EFI: $(BIN) image/EFI/BOOT
	ln -sf ../../../uefi.efi $@

run: image/EFI/BOOT/BOOTX64.EFI
	qemu-system-x86_64 -bios $(FIRMWARE) -drive file=fat:rw:image,media=disk,format=raw
