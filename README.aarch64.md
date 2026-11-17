# AArch64 notes

The aarch64 port runs on QEMU's virt machine (a Cortex-A53), entered in EL1 (EL2 is
dropped to EL1), with a GICv2, the generic timer, a PL011 serial console, a PL031 clock and
virtio-mmio devices.

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

qemu's window shows a ramfb framebuffer, with a virtio keyboard and mouse for the wm; the shell
and kernel debug output are on the serial port. QEMU does not load an initrd for an ELF kernel,
so vmctrl places the ramdisk at the address the loader expects (ARCH_RAMDISK_BASE) with the
generic loader device. The memory pool needs 256 MB.

## Storage and network

./vmctrl run -x aarch64 -S -N

attaches fudge.qcow2 as a virtio-blk device and qemu's user networking (or a tap with -t) as
a virtio-net device, both on virtio-mmio, mounted and set up like on x86. The kernel and
ramdisk still come from the build. Only the eight highest virtio-mmio transports, which QEMU
fills first, are used.

-A adds a virtio-sound device, playing through PulseAudio unless -o names another QEMU audio
driver (pipewire, alsa, or wav to record to fudge.wav); `sound` plays WAV files on it.
