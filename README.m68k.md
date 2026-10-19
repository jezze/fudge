# m68k notes

The m68k port runs on QEMU's m68k virt machine, a 68040 with goldfish devices (interrupt
controllers, timers, tty) and virtio-mmio slots. QEMU enters the kernel in supervisor mode
with the MMU off. The serial shell and the kernel debug output share the goldfish tty.
The wm runs on a virtio-gpu display with a virtio keyboard and mouse.

## Configure and build

make m68k-virt-elf
make

The configuration wants m68k-elf-* tools; a Linux m68k cross compiler works if they are
linked to its m68k-linux-gnu-* tools.

## Run in QEMU

./vmctrl run -x m68k

qemu's window shows the virtio-gpu display; the wm tells the driver what it drew, which
copies that to the host.
QEMU does not load an initrd for an ELF kernel here either, so vmctrl places the ramdisk
at the address the loader expects (ARCH_RAMDISK_BASE) with the generic loader device.
The memory pool needs 256 MB.

## Storage and network

./vmctrl run -x m68k -S -N

attaches fudge.qcow2 as a virtio-blk device and qemu's user networking (or a tap with -t) as
a virtio-net device, mounted and set up like on x86. qemu fills the virtio-mmio slots from the
last one, which is where the bus looks. vmctrl asks for modern virtio-mmio: fudge's rings are
little-endian, and a legacy device here would expect them big-endian.

## Real hardware

Not tried yet. After a page fault the kernel does the writebacks a 68040 leaves in its access fault
frame and goes back through that frame, as the 68040 needs; qemu leaves the writebacks empty, so that
path only runs on a real cpu. The instruction cache is on and invalidated where code is written (a
task's code, a module's relocations). The data cache stays off: device memory would need noncachable
mappings and the drivers would need to invalidate what devices write. The 68080 has an MMU of its own,
so it does not run this yet.
