# ARM notes

The ARM port runs on the Integrator/CP board (ARM926EJ-S) in QEMU. The shell runs on
the first serial port, kernel debug output goes to the second.

## Configure and build

make arm-integratorcp-none
make

## Run in QEMU

./vmctrl run -x arm

The shell is on the terminal, kernel debug output on the second serial port in QEMU's
window. QEMU does not load an initrd for an ELF kernel, so vmctrl places the ramdisk at
the address the loader expects (ARCH_RAMDISK_BASE) with the generic loader device. The
memory pool needs 256 MB.
