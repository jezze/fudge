# m68k notes

The m68k port runs on QEMU's m68k virt machine, a 68040 with goldfish devices (interrupt
controllers, timers, tty) and virtio-mmio slots. QEMU enters the kernel in supervisor mode
with the MMU off. The serial shell and the kernel debug output share the goldfish tty.
The wm runs on a virtio-gpu display with a virtio keyboard and mouse.

## Cross compiler

Build an m68k-unknown-elf cross compiler with crosstool-ng (see README.md) using these
settings in ct-ng menuconfig:

Target options -> Target Architecture -> m68k
Operating System -> Target OS -> bare-metal

Then add it to $PATH:

export PATH=~/x-tools/m68k-unknown-elf/bin:$PATH

A Linux m68k cross compiler works too if its m68k-linux-gnu-* tools are linked as
m68k-unknown-elf-*.

## Configure and build

make m68k-m68kvirt
make

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

-A adds a virtio-sound device, playing through PulseAudio unless -o names another QEMU audio
driver (pipewire, alsa, or wav to record to fudge.wav); `sound` plays WAV files on it.

## Amiga

make m68k-amiga
make

builds fudgeboot as well, an AmigaDOS executable holding the kernel and the ramdisk. Run it
from an AmigaOS or AROS shell (or a Startup-Sequence). It first prints the cpu (exec's
AttnFlags), exec's memory list and Lisa's DENISEID, and returns to the shell with an error
unless the cpu is a 68040 and RAM covers 0x10000000 to 0x1E000000. Otherwise, after 5 seconds,
it takes over the machine through exec's Supervisor(), turns off interrupts, DMA, caches
and the MMU, copies the kernel to its address with the ramdisk after it and enters it; the
loader moves the ramdisk to ARCH_RAMDISK_BASE. The modules are amiga (paula's interrupts, and cia-a's, which it reads
once for every source), amiga-timer (cia-a's timer a at 60 Hz), amiga-serial (paula's
serial port at 115200 baud, the shell's console, shared with kernel debug output),
amiga-keyboard (the keyboard on cia-a's serial port, translated to scancode set 1),
amiga-mouse (the mouse on port 1, read every vertical blank) and amiga-video. There is no
clock or storage yet.

amiga-video shows a pal hires screen, 640x256 in 256 colours (8 bitplanes in chip memory
from 0x00021000 fetched in aga's 4x mode, a copper list at 0x00020000), whatever mode wm
asks for. The palette is a 6x6x6 colour cube with 40 more greys; a 32 KB table gives each
15-bit colour its nearest entry. wm draws into its usual 32-bit framebuffer and each flush
converts that region to planar, 8 pixels at a time as an 8x8 bit transpose.

The kernel's memory starts at ARCH_RAMBASE, 0x10000000 (256 MB of Zorro III RAM where
UAE puts it with its own mapping). So far it runs in FS-UAE as an A4000/040 with these
options:

amiga_model = A4000/040
zorro_iii_memory = 262144
uae_z3mapping = uae
uae_mmu_model = 68040
uae_cpu_compatible = true

UAE only emulates the 68040's MMU with the compatible cpu core. FS-UAE's built-in AROS
ROM is enough to reach a shell; serial_port connects paula to a host device or pty.

## Real hardware

A card or disk the Amiga boots from (amitools provides rdbtool and xdftool):

./vmctrl disk_create -f raw -s 16M -n fudge-amiga
./vmctrl disk_setup_amiga fudge-amiga.raw
dd if=fudge-amiga.raw of=/dev/<card> bs=1M conv=fsync

disk_setup_amiga writes a rigid disk block with one bootable FFS partition (DOS1) holding
fs-uae/hd, so the ROM mounts it with its own filesystem and its boot shell runs fudgeboot from
S/Startup-Sequence. The rest of a larger card stays unused. When fudgeboot refuses, its report
stays on the boot shell's screen.

Not tried yet. After a page fault the kernel does the writebacks a 68040 leaves in its access fault
frame and goes back through that frame, as the 68040 needs; qemu leaves the writebacks empty, so that
path only runs on a real cpu. The instruction cache is on and invalidated where code is written (a
task's code, a module's relocations). The data cache stays off: device memory would need noncachable
mappings and the drivers would need to invalidate what devices write. The 68060's MMU, which the
68080 shares, takes the same tables; its access faults come in a format 4 frame and the cpu restarts
the instruction, with no writebacks. A 68060 traps on the 64-bit mulu.l and muls.l that gcc's
-m68040 code uses, which the 68080 implements.
