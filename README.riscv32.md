# RISC-V (32-bit) notes

The riscv32 port runs on QEMU's virt machine like riscv64 (see README.riscv64.md), entered
in supervisor mode by OpenSBI, with Sv32 paging. The wm runs on a ramfb framebuffer; the
serial shell and the kernel debug output share the serial port.

## Cross compiler

Build a riscv32-unknown-elf cross compiler with crosstool-ng (see README.md) using these
settings in ct-ng menuconfig:

Target options -> Target Architecture -> riscv
Target options -> Bitness -> 32-bit
Operating System -> Target OS -> bare-metal

Then add it to $PATH:

export PATH=~/x-tools/riscv32-unknown-elf/bin:$PATH

## Configure and build

make riscv32-virt
make

## Run in QEMU

./vmctrl run -x riscv32

QEMU's default firmware for riscv32 is opensbi-riscv32-generic-fw_dynamic.bin. Some
distributions (Debian, Ubuntu) leave it out of their qemu packages; QEMU then says it can
not find the RISC-V BIOS. Take the file from pc-bios/ in QEMU's source for your version and
put it in QEMU's data directory (usually /usr/share/qemu).

The ramdisk, the memory layout and the devices are the same as on riscv64.

## Storage and network

./vmctrl run -x riscv32 -S -N

attaches fudge.qcow2 as a virtio-blk device and the network as a virtio-net device, both on
virtio-mmio, like on riscv64, and -A a virtio-sound device.
