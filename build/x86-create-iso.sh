#!/bin/sh

if [ ! -f root/boot/kernel ]
then
    echo "ERROR: Kernel not found. Did you forget to compile?"
    exit
fi

genisoimage -R -b boot/grub/iso9660_stage1_5 -no-emul-boot -boot-load-size 4 -boot-info-table -o fudge.iso root

