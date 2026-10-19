# x86_64 notes

The x86_64 port runs on 64-bit PCs and in QEMU. It is loaded by GRUB as a multiboot2
kernel, under BIOS or UEFI.

## Cross compiler

Build an x86_64-unknown-elf cross compiler with crosstool-ng (see README.md) using these
settings in ct-ng menuconfig:

Target options -> Target Architecture -> x86
Target options -> Bitness -> 64-bit
Operating System -> Target OS -> bare-metal

Then add it to $PATH:

export PATH=~/x-tools/x86_64-unknown-elf/bin:$PATH

## Configure and build

make x86_64-mboot2
make

## Run in QEMU

QEMU can not load a 64-bit kernel directly, so it boots from a disk image (see
README.md) with -S or -d <disk>:

./vmctrl run -x x86_64 -S

or from an ISO made with grub-mkrescue:

make fudge.iso
./vmctrl run -x x86_64 -i fudge.iso

-u boots the disk through UEFI firmware (uefi/ovmf.fd) instead of the BIOS:

./vmctrl run -x x86_64 -S -u

## Storage, network and sound

./vmctrl run -x x86_64 -S -N -A

-s picks the disk controller: ahci, ide, nvme, usb or virtio (the default). -n picks
the network card: virtio (the default) or rtl8139. -A adds an AC97 sound card (-a
virtio for a virtio one), playing through PulseAudio unless -o names another QEMU
audio driver (pipewire, alsa, or wav to record to fudge.wav).

## Real hardware

disk_setup installs GRUB on the image for both BIOS and UEFI, so the image can be
written to a drive. make install copies the kernel, ramdisk and init to /boot for an
existing GRUB; grub/mboot2.cfg shows the entry it needs.
