# m68k notes

The m68k port runs on QEMU's m68k virt machine, a 68040 with goldfish devices (interrupt
controllers, timers, tty) and virtio-mmio slots. QEMU enters the kernel in supervisor mode
with the MMU off. The serial shell and the kernel debug output share the goldfish tty.
There is no display, storage or network yet.

## Configure and build

make m68k-virt-elf
make

The configuration wants m68k-elf-* tools; a Linux m68k cross compiler works if they are
linked to its m68k-linux-gnu-* tools.

## Run in QEMU

./vmctrl run -x m68k

QEMU does not load an initrd for an ELF kernel here either, so vmctrl places the ramdisk
at the address the loader expects (ARCH_RAMDISK_BASE) with the generic loader device.
The memory pool needs 256 MB.

## Real hardware

Page faults rely on QEMU restarting the faulting instruction. A real 68040 also needs the
writebacks pending in its access fault frame done in software, and the 68080 has an MMU of
its own, so neither runs this yet.
