# AArch64 notes

The aarch64 port runs on QEMU's virt machine (a Cortex-A53), entered in EL1 (EL2 is
dropped to EL1). So far it boots the kernel and init with the MMU on; there are no
platform drivers yet, so nothing but kernel debug output reaches the serial port.

## Cross compiler

Build an aarch64-unknown-elf cross compiler with crosstool-ng (see README.md) using these
settings in ct-ng menuconfig:

Target options -> Target Architecture -> arm
Target options -> Bitness -> 64-bit
Operating System -> Target OS -> bare-metal

Then add it to $PATH:

export PATH=~/x-tools/aarch64-unknown-elf/bin:$PATH

## Configure and build

make aarch64-virt
make

## Run in QEMU

./vmctrl run -x aarch64

QEMU does not load an initrd for an ELF kernel, so vmctrl places the ramdisk at the address
the loader expects (ARCH_RAMDISK_BASE) with the generic loader device. Kernel debug output
goes to the serial port. The memory pool needs 256 MB.
