# RISC-V notes

The riscv64 port runs on QEMU's virt machine, entered in supervisor mode by OpenSBI
(QEMU's default firmware). The shell and the kernel debug output share the serial port.

## Configure and build

make riscv64-virt-linux
make

riscv64-virt-elf uses a riscv64-elf toolchain instead of riscv64-linux-gnu.

## Run in QEMU

./vmctrl run -x riscv64

QEMU does not load an initrd for an ELF kernel here either, so vmctrl places the ramdisk
at the address the loader expects (ARCH_RAMDISK_BASE) with the generic loader device.
The memory pool needs 256 MB.

## Storage and network

./vmctrl run -x riscv64 -S -N

attaches fudge.qcow2 as a virtio-blk device and qemu's user networking (or a tap with -t) as
a virtio-net device, both on virtio-mmio, mounted and set up like on x86. The kernel and
ramdisk still come from the build.
