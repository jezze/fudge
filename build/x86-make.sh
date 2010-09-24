ASM="nasm"
ASMFLAGS="-f elf"
GCC="gcc"
GCCFLAGS="-c -O2 -I../include -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -nodefaultlibs"
LD=ld
LDFLAGS=-T./linker-x86.ld

DIR="../kernel"

$GCC $GCCFLAGS $DIR/assert.c -o $DIR/assert.o
$GCC $GCCFLAGS $DIR/elf.c -o $DIR/elf.o
$GCC $GCCFLAGS $DIR/initrd.c -o $DIR/initrd.o
$GCC $GCCFLAGS $DIR/kernel.c -o $DIR/kernel.o
$GCC $GCCFLAGS $DIR/mboot.c -o $DIR/mboot.o
$GCC $GCCFLAGS $DIR/shell.c -o $DIR/shell.o

DIR="../kernel/arch/x86"

$GCC $GCCFLAGS $DIR/arch.c -o $DIR/arch.o
$GCC $GCCFLAGS $DIR/ata.c -o $DIR/ata.o
$ASM $ASMFLAGS $DIR/cr.s -o $DIR/cr.o
$GCC $GCCFLAGS $DIR/fpu.c -o $DIR/fpu.o
$ASM $ASMFLAGS $DIR/fpus.s -o $DIR/fpus.o
$GCC $GCCFLAGS $DIR/gdt.c -o $DIR/gdt.o
$ASM $ASMFLAGS $DIR/gdts.s -o $DIR/gdts.o
$GCC $GCCFLAGS $DIR/idt.c -o $DIR/idt.o
$ASM $ASMFLAGS $DIR/idts.s -o $DIR/idts.o
$ASM $ASMFLAGS $DIR/init.s -o $DIR/init.o
$ASM $ASMFLAGS $DIR/ios.s -o $DIR/io.o
$GCC $GCCFLAGS $DIR/irq.c -o $DIR/irq.o
$ASM $ASMFLAGS $DIR/irqs.s -o $DIR/irqs.o
$GCC $GCCFLAGS $DIR/isr.c -o $DIR/isr.o
$ASM $ASMFLAGS $DIR/isrs.s -o $DIR/isrs.o
$GCC $GCCFLAGS $DIR/kbd.c -o $DIR/kbd.o
$GCC $GCCFLAGS $DIR/mmu.c -o $DIR/mmu.o
$GCC $GCCFLAGS $DIR/pit.c -o $DIR/pit.o
$GCC $GCCFLAGS $DIR/rtc.c -o $DIR/rtc.o
$GCC $GCCFLAGS $DIR/screen.c -o $DIR/screen.o
$GCC $GCCFLAGS $DIR/syscall.c -o $DIR/syscall.o

DIR="../lib"

$GCC $GCCFLAGS $DIR/call.c -o $DIR/call.o
$ASM $ASMFLAGS $DIR/calls.s -o $DIR/calls.o
$GCC $GCCFLAGS $DIR/cbuffer.c -o $DIR/cbuffer.o
$GCC $GCCFLAGS $DIR/memory.c -o $DIR/memory.o
$GCC $GCCFLAGS $DIR/stack.c -o $DIR/stack.o
$GCC $GCCFLAGS $DIR/string.c -o $DIR/string.o
$GCC $GCCFLAGS $DIR/vfs.c -o $DIR/vfs.o

$LD $LDFLAGS \
    ../kernel/assert.o \
    ../kernel/elf.o \
    ../kernel/initrd.o \
    ../kernel/kernel.o \
    ../kernel/mboot.o \
    ../kernel/shell.o \
    ../kernel/arch/x86/arch.o \
    ../kernel/arch/x86/ata.o \
    ../kernel/arch/x86/cr.o \
    ../kernel/arch/x86/fpu.o \
    ../kernel/arch/x86/fpus.o \
    ../kernel/arch/x86/gdt.o \
    ../kernel/arch/x86/gdts.o \
    ../kernel/arch/x86/idt.o \
    ../kernel/arch/x86/idts.o \
    ../kernel/arch/x86/init.o \
    ../kernel/arch/x86/io.o \
    ../kernel/arch/x86/irqs.o \
    ../kernel/arch/x86/isrs.o \
    ../kernel/arch/x86/irq.o \
    ../kernel/arch/x86/isr.o \
    ../kernel/arch/x86/kbd.o \
    ../kernel/arch/x86/mmu.o \
    ../kernel/arch/x86/pit.o \
    ../kernel/arch/x86/rtc.o \
    ../kernel/arch/x86/screen.o \
    ../kernel/arch/x86/syscall.o \
    ../lib/call.o \
    ../lib/calls.o \
    ../lib/cbuffer.o \
    ../lib/memory.o \
    ../lib/stack.o \
    ../lib/string.o \
    ../lib/vfs.o \
    -o root/boot/kernel

ASM="nasm"
ASMFLAGS="-f elf"
GCC="gcc"
GCCFLAGS="-c -O2 -I../include/lib -Wall -ffreestanding -nostdlib -nostartfiles -nodefaultlibs"
LD="ld"
LDFLAGS="-Ttext 0x300000 -e main"

DIR="../ramdisk"

$GCC $GCCFLAGS $DIR/cat.c -o $DIR/cat.o
$LD $LDFLAGS $DIR/cat.o ../lib/memory.o ../lib/string.o ../lib/vfs.o ../lib/call.o ../lib/calls.o -o $DIR/cat
$ASM $ASMFLAGS $DIR/cpu.s -o $DIR/cpus.o
$GCC $GCCFLAGS $DIR/cpu.c -o $DIR/cpu.o
$LD $LDFLAGS $DIR/cpu.o $DIR/cpus.o ../lib/memory.o ../lib/string.o ../lib/vfs.o ../lib/call.o ../lib/calls.o -o $DIR/cpu
$GCC $GCCFLAGS $DIR/date.c -o $DIR/date.o
$LD $LDFLAGS $DIR/date.o ../lib/memory.o ../lib/stack.o ../lib/string.o ../lib/vfs.o ../lib/call.o ../lib/calls.o -o $DIR/date
$GCC $GCCFLAGS $DIR/echo.c -o $DIR/echo.o
$LD $LDFLAGS $DIR/echo.o ../lib/memory.o ../lib/string.o ../lib/vfs.o ../lib/call.o ../lib/calls.o -o $DIR/echo
$GCC $GCCFLAGS $DIR/elf.c -o $DIR/elf.o
$LD $LDFLAGS $DIR/elf.o ../lib/memory.o ../lib/string.o ../lib/vfs.o ../lib/call.o ../lib/calls.o -o $DIR/elf
$GCC $GCCFLAGS $DIR/hello.c -o $DIR/hello.o
$LD $LDFLAGS $DIR/hello.o ../lib/memory.o ../lib/string.o ../lib/vfs.o ../lib/call.o ../lib/calls.o -o $DIR/hello
$GCC $GCCFLAGS $DIR/ls.c -o $DIR/ls.o
$LD $LDFLAGS $DIR/ls.o ../lib/memory.o ../lib/string.o ../lib/vfs.o ../lib/call.o ../lib/calls.o -o $DIR/ls
$GCC $GCCFLAGS $DIR/reboot.c -o $DIR/reboot.o
$LD $LDFLAGS $DIR/reboot.o ../lib/memory.o ../lib/string.o ../lib/vfs.o ../lib/call.o ../lib/calls.o -o $DIR/reboot
$GCC $GCCFLAGS $DIR/shell.c -o $DIR/shell.o
$LD $LDFLAGS $DIR/shell.o ../lib/memory.o ../lib/stack.o ../lib/string.o ../lib/vfs.o ../lib/call.o ../lib/calls.o -o $DIR/shell
$GCC $GCCFLAGS $DIR/timer.c -o $DIR/timer.o
$LD $LDFLAGS $DIR/timer.o ../lib/memory.o ../lib/string.o ../lib/vfs.o ../lib/call.o ../lib/calls.o -o $DIR/timer

GCC="gcc"
GCCFLAGS="-O2"

DIR="../tools"

$GCC $GCCFLAGS $DIR/mkinitrd.c -o $DIR/mkinitrd

../tools/mkinitrd ../ramdisk/about.txt about.txt ../ramdisk/cat cat ../ramdisk/cpu cpu ../ramdisk/date date ../ramdisk/echo echo ../ramdisk/elf elf ../ramdisk/hello hello ../ramdisk/help.txt help.txt ../ramdisk/ls ls ../ramdisk/reboot reboot ../ramdisk/shell shell ../ramdisk/timer timer
