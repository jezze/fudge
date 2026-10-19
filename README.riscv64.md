# RISC-V notes

The riscv64 port runs on QEMU's virt machine, entered in supervisor mode by OpenSBI
(QEMU's default firmware). The wm runs on a ramfb framebuffer; the serial shell and the
kernel debug output share the serial port.

## Cross compiler

Build a riscv64-unknown-elf cross compiler with crosstool-ng (see README.md) using these
settings in ct-ng menuconfig:

Target options -> Target Architecture -> riscv
Target options -> Bitness -> 64-bit
Operating System -> Target OS -> bare-metal

Then add it to $PATH:

export PATH=~/x-tools/riscv64-unknown-elf/bin:$PATH

## Configure and build

make riscv64-virt
make

## Run in QEMU

./vmctrl run -x riscv64

qemu's window shows a ramfb framebuffer, with a virtio keyboard and mouse for the wm.
QEMU does not load an initrd for an ELF kernel here either, so vmctrl places the ramdisk
at the address the loader expects (ARCH_RAMDISK_BASE) with the generic loader device.
The memory pool needs 256 MB.

## Storage and network

./vmctrl run -x riscv64 -S -N

attaches fudge.qcow2 as a virtio-blk device and qemu's user networking (or a tap with -t) as
a virtio-net device, both on virtio-mmio, mounted and set up like on x86. The kernel and
ramdisk still come from the build.

-A adds a virtio-sound device, playing through PulseAudio unless -o names another QEMU audio
driver (pipewire, alsa, or wav to record to fudge.wav); `sound` plays WAV files on it.
