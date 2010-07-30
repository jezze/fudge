#!/bin/sh

dd if=/dev/zero of=fudge.img bs=512 count=8000
dd if=root/boot/grub/stage1 conv=notrunc of=fudge.img bs=512 seek=0
dd if=root/boot/grub/stage2 conv=notrunc of=fudge.img bs=512 seek=1
dd if=root/boot/kernel conv=notrunc of=fudge.img bs=512 seek=200
dd if=root/boot/initrd conv=notrunc of=fudge.img bs=512 seek=300

