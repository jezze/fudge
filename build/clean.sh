#!/bin/sh

rm -f fudge.img
rm -f fudge.iso
rm -f root/bin/*
rm -f root/boot/kernel
rm -f root/boot/initrd
rm -f ../kernel/*.o
rm -f ../arch/arm/kernel/*.o
rm -f ../arch/arm/lib/*.o
rm -f ../arch/x86/kernel/*.o
rm -f ../arch/x86/lib/*.o
rm -f ../lib/*.o
rm -f ../ramdisk/*.o
rm -f ../tools/mkinitrd

