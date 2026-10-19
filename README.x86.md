# x86 notes

The x86 port runs on 32-bit PCs and in QEMU. It is loaded by a multiboot loader such as
GRUB, either as a multiboot kernel or as a multiboot2 kernel. With multiboot2 it also
boots under UEFI.

## Cross compiler

Build an i386-unknown-elf cross compiler with crosstool-ng (see README.md) using these
settings in ct-ng menuconfig:

Target options -> Target Architecture -> x86
Target options -> Bitness -> 32-bit
Operating System -> Target OS -> bare-metal

Then add it to $PATH:

export PATH=~/x-tools/i386-unknown-elf/bin:$PATH

## Configure and build

make x86-mboot
make

or, for multiboot2:

make x86-mboot2
make

## Run in QEMU

./vmctrl run

QEMU loads a multiboot kernel and its ramdisk directly. It can not load a multiboot2
kernel, so an x86-mboot2 build boots from a disk image (see README.md) with -S or
-d <disk>, or from an ISO made with grub-mkrescue:

make fudge.iso
./vmctrl run -i fudge.iso

-u boots the disk through UEFI firmware (uefi/ovmf.fd) instead of the BIOS.

## Storage, network and sound

./vmctrl run -S -N -A

-s picks the disk controller: ahci, ide, nvme, usb or virtio (the default). -n picks
the network card: virtio (the default) or rtl8139. -A adds an AC97 sound card (-a
virtio for a virtio one), playing through PulseAudio unless -o names another QEMU
audio driver (pipewire, alsa, or wav to record to fudge.wav).

## Real hardware

disk_setup installs GRUB on the image for both BIOS and UEFI, so the image can be
written to a drive. make install copies the kernel, ramdisk and init to /boot for an
existing GRUB; grub/mboot.cfg and grub/mboot2.cfg show the entries it needs.
