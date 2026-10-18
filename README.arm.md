# ARM notes

The ARM port runs on the Integrator/CP board (ARM926EJ-S) in QEMU. The shell runs on
the first serial port, kernel debug output goes to the second.

## Configure and build

make arm-integratorcp-none
make

## Run in QEMU

QEMU does not load an initrd for an ELF kernel, so the ramdisk is placed at the address
the loader expects (ARCH_RAMDISK_BASE). The memory pool needs 256 MB.

qemu-system-arm -machine integratorcp -m 256 -kernel fudge -device loader,file=fudge.cpio,addr=0x01200000,force-raw=on -serial stdio -serial file:debug.txt -display none
