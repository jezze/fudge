#!/bin/sh

rm -f fudge.img
rm -f fudge.iso
rm -f root/bin/cat
rm -f root/bin/cd
rm -f root/bin/clear
rm -f root/bin/cpu
rm -f root/bin/date
rm -f root/bin/echo
rm -f root/bin/elf
rm -f root/bin/hello
rm -f root/bin/ls
rm -f root/bin/reboot
rm -f root/bin/timer
rm -f root/bin/vga
rm -f root/boot/kernel
rm -f root/boot/initrd
rm -f ../arch/arm/kernel/*.o
rm -f ../arch/arm/lib/*.o
rm -f ../arch/x86/kernel/*.o
rm -f ../arch/x86/lib/*.o
rm -f ../bin/*.o
rm -f ../kernel/*.o
rm -f ../lib/*.o
rm -f ../tools/mkinitrd

